#include <gio/gio.h>

char *dtools_get_cwd (void);
char *dtools_resolve_path (const char *base, const char *path);
gboolean dtools_path_is_full (const char *path);
gboolean dtools_path_is_absolute (const char *path);
char *dtools_get_bus (const char *path, const char **rest);
gboolean dtools_path_disassemble (const char *path,
				  GDBusConnection **connection,
				  char **name,
				  char **objpath,
				  char **interface,
				  GError **error);
