#include <gio/gio.h>

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
  
  g_type_init ();

  writer = dtools_variant_writer_new (STDOUT_FILENO);
  
  /* Read parameters */
  for (n = 1; n < argc; n++)
    {
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
              return 1;
            }
        }

      dtools_variant_writer_add (writer, value);
      g_variant_unref (value);
    }
  
  return 0;
}
