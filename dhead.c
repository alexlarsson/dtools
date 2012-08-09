#include <gio/gio.h>

#include "dtools.h"

int
main (int argc, char *argv[])
{
  GVariant *value;
  DtoolsVariantWriter *writer;
  DtoolsVariantReader *reader;
  int n_items, i;
  
  g_type_init ();

  writer = dtools_variant_writer_new (STDOUT_FILENO);
  reader = dtools_variant_reader_new (STDIN_FILENO);

  if (argc >= 2)
    n_items = atoi (argv[1]);
  else
    n_items = 10;

  i = 0;
  while (i < n_items)
    {
      value = dtools_variant_reader_next (reader);

      if (value == NULL)
	break;

      dtools_variant_writer_add (writer, value);
      i++;
    }

  return 0;
}
