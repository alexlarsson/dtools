#include <gio/gio.h>
#include <stdlib.h>

#include "dtools.h"

gchar *
escape_unicode (const gchar *str)
{
  const gchar *p;
  GString *dest;
  gunichar c;

  g_return_val_if_fail (str != NULL, NULL);
  
  dest = g_string_new ("");

  p = str;

  while (*p)
    {
      c = g_utf8_get_char (p);
      
      /* Replace literal ' with a close ', a \', and a open ' */
      if (c < 0xff && g_ascii_isprint (c))
        g_string_append_unichar (dest, c);
      else if (c < 0xffff)
        g_string_append_printf (dest, "\\u%04x", (int)c);
      else
        g_string_append_printf (dest, "\\U%08x", (int)c);

      p = g_utf8_next_char (p);
    }
  
  return g_string_free (dest, FALSE);
}

int
main (int argc, char *argv[])
{
  GVariant *value;
  DtoolsVariantReader *reader;
  char *str, *escaped;
  int arg_nr, i;
  
  g_type_init ();

  reader = dtools_variant_reader_new (STDIN_FILENO);

  arg_nr = 0;
  if (argc >= 2)
    arg_nr = atoi (argv[1]);

  value = NULL;
  for (i = 0; i <= arg_nr; i++)
    {
      value = dtools_variant_reader_next (reader);
      if (value != NULL && i + 1 < arg_nr)
	g_variant_unref (value);
    }
  if (value)
    {
      str = g_variant_print (value, TRUE);
      escaped = escape_unicode (str);
      g_print ("%s\n", escaped);
      g_free (str);
      g_free (escaped);
      g_variant_unref (value);
    }
  
  return 0;
}
