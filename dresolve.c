#include <gio/gio.h>

#include "dtools.h"

int
main (int argc, char *argv[])
{
  const char *cwd;
  char *path;

  cwd = dtools_get_cwd ();

  if (argc > 1)
    path = dtools_resolve_path (cwd, argv[1]);
  else
    path = dtools_resolve_path (cwd, NULL);

  g_print ("%s\n", path);

  return 0;
}
