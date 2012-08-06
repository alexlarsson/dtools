#include <string.h>
#include <gio/gio.h>

gboolean
dtools_path_is_full (const char *path)
{
  return path[0] == '/' && path[1] == '/';
}

gboolean
dtools_path_is_absolute (const char *path)
{
  return path[0] == '/';
}

char *
dtools_get_bus (const char *path, const char **rest)
{
  const char *start, *p;

  g_return_val_if_fail (path != NULL, NULL);
  g_return_val_if_fail (dtools_path_is_full (path), NULL);

  start = path;
  while (*start == '/')
    start++;

  p = start;
  while (*p != 0 && *p != '/')
    p++;

  if (rest)
    *rest = p;

  return g_strndup (start - 2, 2 + p - start);
}

static char *
canonicalize (const char *path)
{
  GString *s = g_string_new ("//");
  int start;
  const char *element, *end;

  /* Skip initial slashes */
  while (*path != 0 && *path == '/') {
    path++;
  }

  /* Append bus name */
  while (*path != 0 && *path != '/') {
    g_string_append_c (s, *path++);
  }

  start = s->len;

  while (*path != 0) {
    /* There was another path element */
    while (*path != 0 && *path == '/') {
      path++;
    }

    if (*path != 0) {
      /* And its not empty */
      element = path;

      while (*path != 0 && *path != '/') {
	path++;
      }

      end = path;

      if (end - element == 1 &&
	  strncmp (element, ".", 1) == 0) {
	/* Skip */
      } else if (end - element == 2 &&
		 strncmp (element, "..", 2) == 0) {
	/* Go back an element, unless at start */
	if (s->len > start)
	  {
	    int len = s->len;
	    while (s->str[len-1] != '/')
	      len--;
	    g_string_truncate (s, len-1);
	  }
      } else {
	g_string_append_c (s, '/');
	g_string_append_len (s, element, end - element);
      }
    }
  }

  return g_string_free (s, FALSE);
}

char *
dtools_resolve_path (const char *base, const char *path)
{
  const char *base_rest;
  char *new_path, *res;

  g_return_val_if_fail (base != NULL, NULL);
  g_return_val_if_fail (dtools_path_is_full (base), NULL);

  if (path == NULL)
    return g_strdup (base);

  if (dtools_path_is_full (path))
    return canonicalize (path);

  if (dtools_path_is_absolute (path))
    {
      char *bus = dtools_get_bus (base, &base_rest);
      new_path = g_strconcat (bus, path, NULL);
      g_free (bus);
    }
  else
    new_path = g_build_path ("/", base, path, NULL);

  res = canonicalize (new_path);
  g_free (new_path);
  return res;

}


char *
dtools_get_cwd (void)
{
  const char *path;

  path = g_getenv ("DPATH");
  if (path != NULL && *path != 0)
    return dtools_resolve_path ("//session", path);

  return g_strdup ("//session");
}


GBusType
dtools_get_bus_type (const char *bus)
{
  while (*bus == '/')
    bus++;

  if (strcmp (bus, "session") == 0)
    return G_BUS_TYPE_SESSION;
  if (strcmp (bus, "system") == 0)
    return G_BUS_TYPE_SYSTEM;

  return G_BUS_TYPE_NONE;
}

gboolean
dtools_path_disassemble (const char *path,
			 GDBusConnection **connection,
			 char **name,
			 char **objpath,
			 char **interface,
			 GError **error)
{
  const char *rest, *s, *last;
  char *bus;
  GBusType type;

  bus = dtools_get_bus (path, &rest);
  type = dtools_get_bus_type (bus);
  g_free (bus);

  *connection = g_bus_get_sync (type, NULL, error);
  if (*connection == NULL)
    return FALSE;

  *name = NULL;
  *objpath = NULL;
  *interface = NULL;

  if (*rest != 0)
    {
      while (*rest == '/')
	rest++;
      s = rest;
      while (*rest != 0 && *rest != '/')
	rest++;
      *name = g_strndup (s, rest - s);

      if (*rest != 0)
	{
	  last = strrchr (rest, '/');
	  if (last != NULL && strchr (last, '.') != NULL)
	    {
	      *interface = g_strdup (last + 1);
	      if (last == rest)
		*objpath = g_strdup ("/");
	      else
		*objpath = g_strndup (rest, last - rest);
	    }
	  else
	    *objpath = g_strdup (rest);
	}
      else
	*objpath = g_strdup ("/");
    }
  return TRUE;
}
