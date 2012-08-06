#include <gio/gio.h>
#include <string.h>

#include "dtools.h"

static GVariant *
_g_variant_parse_me_harder (GVariantType   *type,
                            const gchar    *given_str,
                            GError        **error)
{
  GVariant *value;
  gchar *s;
  guint n;
  GString *str;

  str = g_string_new ("\"");
  for (n = 0; given_str[n] != '\0'; n++)
    {
      if (G_UNLIKELY (given_str[n] == '\"'))
        g_string_append (str, "\\\"");
      else
        g_string_append_c (str, given_str[n]);
    }
  g_string_append_c (str, '"');
  s = g_string_free (str, FALSE);

  value = g_variant_parse (type,
                           s,
                           NULL,
                           NULL,
                           error);
  g_free (s);

  return value;
}

int
main (int argc, char *argv[])
{
  guint n;
  GError *error;
  GVariant *value;
  DtoolsVariantWriter *writer;
  GDBusConnection *connection;
  GVariantBuilder builder;
  const char *cwd;
  gsize n_children;
  char *path, *objpath, *name, *interface, *method, *p;
  GVariant *parameters, *result;

  g_type_init ();

  /* Do this early to avoid spinning in writer */
  writer = dtools_variant_writer_new (STDOUT_FILENO);
  
  if (argc < 2)
    {
      g_printerr ("No method specified\n");
      return 1;
    }

  cwd = dtools_get_cwd ();

  path = dtools_resolve_path (cwd, argv[1]);

  error = NULL;
  if (!dtools_path_disassemble (path, &connection, &name, &objpath, &interface, &error))
    {
      g_printerr ("Error: %s\n", error->message);
      return 1;
    }

  if (interface == NULL)
    {
      g_printerr ("Not an interface: %s\n", error->message);
      return 1;
    }

  p = strrchr (interface, '.');
  g_assert (p != NULL); // Valid interfaces have at least one dot

  *p = 0;
  method = p + 1;

  /* TODO: Introspect arg types for easier arg specification */

  
  /* Read parameters */
  g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);
  for (n = 2; n < argc; n++)
    {
      /* TODO: Move this to dtools.c */
      error = NULL;
      value = g_variant_parse (NULL,
                               argv[n],
                               NULL,
                               NULL,
                               &error);
      if (value == NULL)
        {
          g_error_free (error);
          error = NULL;
          value = _g_variant_parse_me_harder (NULL, argv[n], &error);
          if (value == NULL)
            {
	      g_printerr ("Error parsing parameter %d: %s\n",
			  n,
			  error->message);
              g_error_free (error);
              g_variant_builder_clear (&builder);
              return 1;
            }
        }

      g_variant_builder_add_value (&builder, value);
    }
  parameters = g_variant_builder_end (&builder);

  if (parameters != NULL)
    parameters = g_variant_ref_sink (parameters);
  result = g_dbus_connection_call_sync (connection,
                                        name,
                                        objpath,
                                        interface,
                                        method,
                                        parameters,
                                        NULL,
                                        G_DBUS_CALL_FLAGS_NONE,
                                        30*1000 /* timeout */,
                                        NULL,
                                        &error);
  if (result == NULL)
    {
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);
      /*
      if (in_signature_types != NULL)
        {
          GString *s;
          s = g_string_new (NULL);
          for (n = 0; n < in_signature_types->len; n++)
            {
              GVariantType *type = in_signature_types->pdata[n];
              g_string_append_len (s,
                                   g_variant_type_peek_string (type),
                                   g_variant_type_get_string_length (type));
            }
          g_printerr ("(According to introspection data, you need to pass `%s')\n", s->str);
          g_string_free (s, TRUE);
        }
      */
      return 1;
    }
  
  n_children = g_variant_n_children (result);
  for (n = 0; n < n_children; n++)
    {
      value = g_variant_get_child_value (result, n);
      dtools_variant_writer_add (writer, value);
      g_variant_unref (value);
    }
  
  return 0;
}
