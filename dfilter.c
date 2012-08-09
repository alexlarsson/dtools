#include <stdlib.h>
#include <gio/gio.h>

#include "dtools.h"

static char *filter_index = NULL;
static char comparison;
static GVariant *cmp_value = NULL;

static GVariant *
find_child (GVariant *v)
{
  GVariantClass v_class;
  GVariant *c;

  if (filter_index == NULL)
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
      return g_variant_lookup_value (v, filter_index, NULL);
    }

  g_warning ("Unsupported sort index type");
  return v;
}

int
main (int argc, char *argv[])
{
  GVariant *value, *c;
  DtoolsVariantWriter *writer;
  DtoolsVariantReader *reader;
  int cmp_res;
  
  g_type_init ();

  writer = dtools_variant_writer_new (STDOUT_FILENO);
  reader = dtools_variant_reader_new (STDIN_FILENO);

  if (argc < 4)
    {
      g_printerr ("not enough args\n");
      return 1;
    }
  
  filter_index = argv[1];
  comparison = argv[2][0];
  if (comparison != '<' &&
      comparison != '=' &&
      comparison != '>')
    {
      g_printerr ("wrong comparison\n");
      return 1;
    }
      
  cmp_value = g_variant_new_int64 (atoi (argv[3]));

  /* Read input */
  while (TRUE)
    {
      value = dtools_variant_reader_next (reader);

      if (value == NULL)
	break;

      c = find_child (value);
      cmp_res = dtools_compare_variant (cmp_value, c);

      switch (comparison) {
      case '<':
	if (cmp_res > 0)
	  dtools_variant_writer_add (writer, value);
	break;
      case '=':
	if (cmp_res == 0)
	  dtools_variant_writer_add (writer, value);
	break;
      case '>':
	if (cmp_res < 0)
	  dtools_variant_writer_add (writer, value);
	break;
      }
    }

  return 0;
}
