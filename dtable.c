#include <gio/gio.h>

#include "dtools.h"

int
main (int argc, char *argv[])
{
  GVariant *value;
  DtoolsVariantReader *reader;
  int i;
  
  g_type_init ();

  reader = dtools_variant_reader_new (STDIN_FILENO);

  for (i = 1; i < argc; i++)
    {
      if (i > 1)
	g_print (" ");
      g_print ("%8s", argv[i]);
    }
  g_print ("\n");
  
  /* Read input */
  while (TRUE)
    {
      value = dtools_variant_reader_next (reader);

      if (value == NULL)
	break;

      for (i = 1; i < argc; i++)
	{
	  GVariant *c;
	  char *str;

	  if (i > 1)
	    g_print (" ");
	    
	  c = g_variant_lookup_value (value, argv[i], NULL);
	  str = g_variant_print (c, FALSE);
	  g_print ("%8s", str);
	  g_free (str);
	}
      g_print ("\n");
      
      g_variant_unref (value);
    }
  
  return 0;
}
