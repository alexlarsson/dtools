/*
 * Copyright © 2012 Alexander Larsson <alexl@redhat.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gio/gio.h>

#include "pipeneg.h"
#include "variant-writer.h"

struct DtoolsVariantWriter
{
  GObject parent_instance;
  int stream_format;
  int fd;
};
  
G_DEFINE_TYPE (DtoolsVariantWriter, dtools_variant_writer, G_TYPE_OBJECT)

static void
dtools_variant_writer_init (DtoolsVariantWriter *self)
{
  self->stream_format = -1;
}

static void
dtools_variant_writer_finalize (GObject *object)
{
  G_OBJECT_CLASS (dtools_variant_writer_parent_class)->finalize (object);
}

static void
dtools_variant_writer_class_init (DtoolsVariantWriterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dtools_variant_writer_finalize;
}

DtoolsVariantWriter *
dtools_variant_writer_new (int fd)
{
  DtoolsVariantWriter *writer;

  writer = g_object_new (DTOOLS_TYPE_VARIANT_WRITER, NULL);
  writer->fd = fd;

  return writer;
}

static void
write_all (DtoolsVariantWriter *writer,
	   const char *data, gsize len)
{
  gsize written;
  gssize res;
  
  written = 0;
  while (len > written)
    {
      do
	res = write (writer->fd, data + written, len - written);
      while (res < 0 && errno == EINTR);

      if (res <= 0)
	{
	  g_warning ("Error writing variant\n");
	  break;
	}
      else
	written += res;
    }
}

void
dtools_variant_writer_add (DtoolsVariantWriter *writer,
			   GVariant *value)
{
  char *str, *orig_str;
  gsize len;
  GVariant *variant;
  gboolean was_first;

  variant = g_variant_new_variant (value);

  was_first = FALSE;
  if (writer->stream_format == -1)
    {
      writer->stream_format = pipe_negotiate_format (writer->fd);
      was_first = TRUE;
    }

  if (writer->stream_format == FORMAT_TEXT)
    {
      orig_str = str = g_variant_print (variant, FALSE);

      if (was_first)
	str++;
  
      len = strlen (str);

      write_all (writer, str, len);
      write_all (writer, "\n", 1);

      g_free (orig_str);
    }
  else
    {
      GVariant *normal;
      const gchar *data;
      gsize size;
      guint32 size_le;

      /* Mark this as a binary stream */
      if (was_first)
	write_all (writer, "\0", 1);

      normal = g_variant_get_normal_form (variant);
      size = g_variant_get_size (normal);
      if (size > G_MAXUINT32)
	g_error ("Too large variant");

      size_le = GUINT32_TO_LE ((guint32)size);
      write_all (writer, (char *)&size_le, 4);

      data = g_variant_get_data (normal);
      write_all (writer, data, size);

      g_variant_unref (normal);
    }

  g_variant_unref (variant);
}
