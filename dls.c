#include <gio/gio.h>

#include <string.h>
#include "dtools.h"

static void
print_names (GDBusConnection *c,
	     gboolean         include_unique_names)
{
  GVariant *result;
  GError *error;
  GVariantIter *iter;
  gchar *str;
  GHashTable *name_set;
  GList *keys;
  GList *l;

  name_set = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  error = NULL;
  result = g_dbus_connection_call_sync (c,
					"org.freedesktop.DBus",
					"/org/freedesktop/DBus",
					"org.freedesktop.DBus",
					"ListNames",
					NULL,
					G_VARIANT_TYPE ("(as)"),
					G_DBUS_CALL_FLAGS_NONE,
					3000, /* 3 secs */
					NULL,
					&error);
  if (result == NULL)
    {
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);
      goto out;
    }
  g_variant_get (result, "(as)", &iter);
  while (g_variant_iter_loop (iter, "s", &str))
    g_hash_table_insert (name_set, g_strdup (str), NULL);
  g_variant_iter_free (iter);
  g_variant_unref (result);

  error = NULL;
  result = g_dbus_connection_call_sync (c,
					"org.freedesktop.DBus",
					"/org/freedesktop/DBus",
					"org.freedesktop.DBus",
					"ListActivatableNames",
					NULL,
					G_VARIANT_TYPE ("(as)"),
					G_DBUS_CALL_FLAGS_NONE,
					3000, /* 3 secs */
					NULL,
					&error);
  if (result == NULL)
    {
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);
      goto out;
    }
  g_variant_get (result, "(as)", &iter);
  while (g_variant_iter_loop (iter, "s", &str))
    g_hash_table_insert (name_set, g_strdup (str), NULL);
  g_variant_iter_free (iter);
  g_variant_unref (result);

  keys = g_hash_table_get_keys (name_set);
  keys = g_list_sort (keys, (GCompareFunc) g_strcmp0);
  for (l = keys; l != NULL; l = l->next)
    {
      const gchar *name = l->data;
      if (!include_unique_names && g_str_has_prefix (name, ":"))
	continue;

      g_print ("%s \n", name);
    }
  g_list_free (keys);

 out:
  g_hash_table_unref (name_set);
}

static void
print_paths (GDBusConnection *c,
	     const gchar *name,
	     const gchar *path,
	     const gchar *interface)
{
  GVariant *result;
  GError *error;
  const gchar *xml_data;
  GDBusNodeInfo *node;
  guint n;

  error = NULL;
  result = g_dbus_connection_call_sync (c,
					name,
					path,
					"org.freedesktop.DBus.Introspectable",
					"Introspect",
					NULL,
					G_VARIANT_TYPE ("(s)"),
					G_DBUS_CALL_FLAGS_NONE,
					3000, /* 3 secs */
					NULL,
					&error);
  if (result == NULL)
    {
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);
      goto out;
    }
  g_variant_get (result, "(&s)", &xml_data);

  error = NULL;
  node = g_dbus_node_info_new_for_xml (xml_data, &error);
  g_variant_unref (result);
  if (node == NULL)
    {
      g_printerr ("Error parsing introspection XML: %s\n", error->message);
      g_error_free (error);
      goto out;
    }

  if (interface == NULL)
    {
      for (n = 0; node->nodes != NULL && node->nodes[n] != NULL; n++)
	g_print ("%s\n", node->nodes[n]->path);
    }

  for (n = 0; node->interfaces != NULL && node->interfaces[n] != NULL; n++)
    {
      const GDBusInterfaceInfo *iface = node->interfaces[n];
      if (interface == NULL)
	g_print ("%s\n", iface->name);
      else if (strcmp (iface->name, interface) == 0)
	{
	  int m;
	  for (m = 0; iface->methods != NULL && iface->methods[m] != NULL; m++)
	    {
	      const GDBusMethodInfo *method = iface->methods[m];
	      g_print ("%s\n", method->name);
	    }
	}
    }

  g_dbus_node_info_unref (node);

 out:
  ;
}



int
main (int argc, char *argv[])
{
  GDBusConnection *connection;
  const char *cwd;
  char *path, *objpath, *name, *interface;
  GError *error;

  g_type_init ();

  cwd = dtools_get_cwd ();

  if (argc > 1)
    path = dtools_resolve_path (cwd, argv[1]);
  else
    path = dtools_resolve_path (cwd, NULL);

  error = NULL;
  if (!dtools_path_disassemble (path, &connection, &name, &objpath, &interface, &error))
    {
      g_printerr ("Error: %s\n", error->message);
      return 1;
    }

  if (name == NULL)
    {
      print_names (connection, FALSE);
    }
  else
    {
      print_paths (connection, name, objpath, interface);
    }

  return 0;
}
