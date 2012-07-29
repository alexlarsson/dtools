#include <gio/gio.h>

char *dtools_get_cwd (void);
char *dtools_resolve_path (const char *base, const char *path);
gboolean dtools_path_is_full (const char *path);
gboolean dtools_path_is_absolute (const char *path);
