#include <gio/gio.h>

#include "dtools.h"

int
main (int argc, char *argv[])
{
  const char *cwd;

  cwd = dtools_get_cwd ();
  g_print ("%s\n", cwd);

  return 0;
}
