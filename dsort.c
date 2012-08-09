#include <gio/gio.h>

#include "dtools.h"

static char *sort_index = NULL;

static GVariant *
find_child (GVariant *v)
{
  GVariantClass v_class;
  GVariant *c;

  if (sort_index == NULL)
    return v;
  
  v_class = g_variant_classify (v);

  if (v_class == G_VARIANT_CLASS_MAYBE)
    {
      c = g_variant_get_maybe (v);
      if (c == NULL)
	return NULL;
      return find_child (c);
    }

  if (v_class == G_VARIANT_CLASS_VARIANT)
    {
      c = g_variant_get_child_value (v, 0);
      return find_child (c);
    }
  
  if (g_variant_is_of_type (v, G_VARIANT_TYPE ("a{s*}")))
    {
      return g_variant_lookup_value (v, sort_index, NULL);
    }

  g_warning ("Unsupported sort index type");
  return v;
}

static int
compare (gconstpointer a, gconstpointer b)
{
  GVariant *aa = (GVariant *)a;
  GVariant *bb = (GVariant *)b;

  
  
  return -dtools_compare_variant (find_child (aa),
				  find_child (bb));
}

int
main (int argc, char *argv[])
{
  GVariant *value;
  DtoolsVariantWriter *writer;
  DtoolsVariantReader *reader;
  GList *vs, *l;
  
  g_type_init ();

  writer = dtools_variant_writer_new (STDOUT_FILENO);
  reader = dtools_variant_reader_new (STDIN_FILENO);

  if (argc >= 2)
    sort_index = argv[1];

  vs = NULL;
  /* Read input */
  while (TRUE)
    {
      value = dtools_variant_reader_next (reader);

      if (value == NULL)
	break;

      vs = g_list_prepend (vs, value);
    }

  vs = g_list_sort (vs, compare);

  for (l = vs; l != NULL; l = l->next)
    {
      value = l->data;
      dtools_variant_writer_add (writer, value);
      g_variant_unref (value);
    }

  g_list_free (vs);

  return 0;
}
