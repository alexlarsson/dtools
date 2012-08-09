#include <string.h>
#include <math.h>
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

static gboolean
variant_class_is_int (GVariantClass v_class)
{
 return
   v_class == G_VARIANT_CLASS_BYTE ||
   v_class == G_VARIANT_CLASS_INT16 ||
   v_class == G_VARIANT_CLASS_UINT16 ||
   v_class == G_VARIANT_CLASS_INT32 ||
   v_class == G_VARIANT_CLASS_UINT32 ||
   v_class == G_VARIANT_CLASS_INT64 ||
   v_class == G_VARIANT_CLASS_UINT64;
}


static gboolean
variant_class_is_signed_int (GVariantClass v_class)
{
 return
   v_class == G_VARIANT_CLASS_INT16 ||
   v_class == G_VARIANT_CLASS_INT32 ||
   v_class == G_VARIANT_CLASS_INT64;
}

static gboolean
variant_as_int64 (GVariant *v, GVariantClass v_class, gint64 *as_int)
{
  guint64 u64;

  switch (v_class)
    {
    case G_VARIANT_CLASS_BYTE:
      *as_int = (gint64) g_variant_get_byte (v);
      return TRUE;
    case G_VARIANT_CLASS_INT16:
      *as_int = g_variant_get_int16 (v);
      return TRUE;
    case G_VARIANT_CLASS_UINT16:
      *as_int = g_variant_get_uint16 (v);
      return TRUE;
    case G_VARIANT_CLASS_INT32:
      *as_int = g_variant_get_int32 (v);
      return TRUE;
    case G_VARIANT_CLASS_UINT32:
      *as_int = g_variant_get_uint32 (v);
      return TRUE;
    case G_VARIANT_CLASS_INT64:
      *as_int = g_variant_get_int64 (v);
      return TRUE;
    case G_VARIANT_CLASS_UINT64:
      u64 = g_variant_get_uint64 (v);
      if (u64 <= G_MAXINT64)
	{
	  *as_int = u64;
	  return TRUE;
	}
      return FALSE;
    default:
      return FALSE;
    }
}

static gboolean
variant_as_uint64 (GVariant *v, GVariantClass v_class, guint64 *as_uint)
{
  gint64 i64;

  switch (v_class)
    {
    case G_VARIANT_CLASS_BYTE:
      *as_uint = (gint64) g_variant_get_byte (v);
      return TRUE;
    case G_VARIANT_CLASS_UINT16:
      *as_uint = g_variant_get_uint16 (v);
      return TRUE;
    case G_VARIANT_CLASS_UINT32:
      *as_uint = g_variant_get_uint32 (v);
      return TRUE;
    case G_VARIANT_CLASS_UINT64:
      *as_uint = g_variant_get_uint64 (v);
      return FALSE;
    case G_VARIANT_CLASS_INT16:
      i64 = g_variant_get_int16 (v);
      if (i64 > 0)
	{
	  *as_uint = i64;
	  return TRUE;
	}
      return FALSE;
    case G_VARIANT_CLASS_INT32:
      i64 = g_variant_get_int32 (v);
      if (i64 > 0)
	{
	  *as_uint = i64;
	  return TRUE;
	}
      return FALSE;
    case G_VARIANT_CLASS_INT64:
      i64 = g_variant_get_int64 (v);
      if (i64 > 0)
	{
	  *as_uint = i64;
	  return TRUE;
	}
      return FALSE;
    default:
      return FALSE;
    }
}

static int
compare_double_int64 (double a, gint64 b)
{
  gint64 a_floor, a_ceil;
    
  /* Its not generally possible to just cast a 64bit int
     to a double, as double only has 52bit mantissa, nor is
     it safe to cast a double to an int as you then throw away
     the non integer portion. What we do is compare both
     the ceil and the floor, which lets use also compare
     the fractional part. */

  if (a < G_MININT64)
    return -1;
  if (a > G_MAXINT64)
    return 1;

  a_ceil = (gint64)ceil (a);
  if (a_ceil < b)
    return -1;
  a_floor = (gint64)floor (a);
  if (a_ceil == b)
    {
      if (a_floor == a_ceil)
	return 0; /* a is really an integer, so really equal */
      return -1;
    }
  return 1;
}

static int
compare_double_uint64 (double a, guint64 b)
{
  guint64 a_floor, a_ceil;

  /* Its not generally possible to just cast a 64bit int
     to a double, as double only has 52bit mantissa, nor is
     it safe to cast a double to an int as you then throw away
     the non integer portion. What we do is compare both
     the ceil and the floor, which lets use also compare
     the fractional part. */

  if (a < 0)
    return -1;
  if (a > G_MAXUINT64)
    return 1;

  a_ceil = (guint64)ceil (a);
  if (a_ceil < b)
    return -1;
  a_floor = (guint64)floor (a);
  if (a_ceil == b)
    {
      if (a_floor == a_ceil)
	return 0; /* a is really an integer, so really equal */
      return -1;
    }
  return 1;
}

/* Generic qsort style comparison of variants
   For variants of the same class the "natural" order is used.
   If the types differ, we compare by value of GVariantClass
   except for:
    All numeric types (ints, double) compare by value
    All textish string types compare by utf8 collation
*/   
int
dtools_compare_variant (GVariant *a,
			GVariant *b)
{
  gint64 a64, b64;
  guint64 au64, bu64;
  double ad, bd;
  GVariantClass a_class, b_class;
  GVariant *ma, *mb;

  if (a == NULL && b == NULL)
    return 0;
  if (a == NULL)
    return 1;
  if (b == NULL)
    return -1;
  
  a_class = g_variant_classify (a);
  b_class = g_variant_classify (b);

  if (a_class == G_VARIANT_CLASS_MAYBE)
    {
      ma = g_variant_get_maybe (a);
      if (ma == NULL)
	{
	  if (b_class == G_VARIANT_CLASS_MAYBE &&
	      g_variant_get_maybe (b) == NULL)
	    return 0;
	  else
	    return 1; /* NULL sorts at end */
	}
      return dtools_compare_variant (ma, b);
    }

  if (b_class == G_VARIANT_CLASS_MAYBE)
    {
      mb = g_variant_get_maybe (b);
      if (mb == NULL)
	return -1; /* NULL sorts at end */
      return dtools_compare_variant (a, mb);
    }
  
  if (a_class == G_VARIANT_CLASS_VARIANT)
    return dtools_compare_variant (g_variant_get_child_value (a, 0),
				   b);
  
  if (b_class == G_VARIANT_CLASS_VARIANT)
    return dtools_compare_variant (a,
				   g_variant_get_child_value (b, 0));

  if (a_class == G_VARIANT_CLASS_DOUBLE &&
      b_class == G_VARIANT_CLASS_DOUBLE)
    {
      ad = g_variant_get_double (a);
      bd = g_variant_get_double (b);

      if (ad < bd)
	return -1;
      if (ad == bd)
	return 0;
      else
	return 1;
    }
  
  if (variant_class_is_int (a_class) &&
      variant_class_is_int (b_class))
    {
      if (variant_as_int64 (a, a_class, &a64) &&
	  variant_as_int64 (b, b_class, &b64))
	{
	  if (a64 < b64)
	    return -1;
	  else if (a64 == b64)
	    return 0;
	  else
	    return 1;
	}

      /* For integers the only leftover cases are:
	 a) Both a & b are uint64 (> maxint64)
	 b) One of a/b is uint64 (> maxint64)
	 b1) The other fits in uint64
	 b2) The other is < 0

	 This handles everything but case b2:
      */
      if (variant_as_uint64 (a, a_class, &au64) &&
	  variant_as_uint64 (b, b_class, &bu64))
	{
	  if (au64 < bu64)
	    return -1;
	  else if (au64 == bu64)
	    return 0;
	  else
	    return 1;
	}

      /* Handle case b2, one uint64, other is < 0  */
      if (variant_class_is_signed_int (a_class))
	/* signed but doesn't fit in uint64 => negative */
	return -1;
      else if (variant_class_is_signed_int (b_class))
	/* signed but doesn't fit in uint64 => negative */
	return 1;

      /* Can't be equal and yet not fit in either uint64 or int64 */
      g_assert_not_reached ();
    }

  if (a_class == G_VARIANT_CLASS_DOUBLE &&
      variant_class_is_int (b_class))
    {
      ad = g_variant_get_double (a);

      if (variant_as_int64 (b, b_class, &b64))
	return compare_double_int64 (ad, b64);
      else
	{
	  variant_as_uint64 (b, b_class, &bu64);
	  return compare_double_uint64 (ad, bu64);
	}
    }

  if (variant_class_is_int (a_class) &&
      b_class == G_VARIANT_CLASS_DOUBLE)
    {
      bd = g_variant_get_double (b);

      if (variant_as_int64 (a, a_class, &a64))
	return -compare_double_int64 (bd, a64);
      else
	{
	  variant_as_uint64 (a, a_class, &au64);
	  return -compare_double_uint64 (bd, au64);
	}

    }

  g_warning ("Unhandled variant type in compare\n");

  /* TODO: We just sort by ptr for now */
  if ((gsize)a < (gsize)b)
    return -1;
  else if (a == b)
    return 0;
  else
    return 1;
}
