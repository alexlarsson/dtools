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
#include "variant-reader.h"

struct DtoolsVariantReader
{
  GObject parent_instance;
  int stream_format;
  int fd;
  GString *buffer;
  gsize offset;
};
  
G_DEFINE_TYPE (DtoolsVariantReader, dtools_variant_reader, G_TYPE_OBJECT)

static void
dtools_variant_reader_init (DtoolsVariantReader *self)
{
  self->stream_format = -1;
  self->buffer = g_string_sized_new (8*1024);
}

static void
dtools_variant_reader_finalize (GObject *object)
{
  G_OBJECT_CLASS (dtools_variant_reader_parent_class)->finalize (object);
}

static void
dtools_variant_reader_class_init (DtoolsVariantReaderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dtools_variant_reader_finalize;
}

DtoolsVariantReader *
dtools_variant_reader_new (int fd)
{
  DtoolsVariantReader *reader;

  reader = g_object_new (DTOOLS_TYPE_VARIANT_READER, NULL);
  reader->fd = fd;

  pipe_setup_for_binary (fd);

  return reader;
}

static gboolean
read_some (DtoolsVariantReader *reader)
{
  gssize res;

  if (reader->buffer->allocated_len - (reader->buffer->len + 1) < 4*1024)
    {
      gsize old_size = reader->buffer->len;
      g_string_set_size (reader->buffer, reader->buffer->allocated_len + 4*1024);
      g_string_set_size (reader->buffer, old_size);
    }

  do
    {
      res = read (reader->fd, reader->buffer->str + reader->buffer->len,
		  reader->buffer->allocated_len - (reader->buffer->len + 1));
    }
  while (res < 0 && errno == EINTR);

  if (res == 0)
    return FALSE;
  
  if (res < 0)
    g_error ("Error reading input\n");
  else if (res > 0)
    {
      g_string_set_size (reader->buffer, reader->buffer->len + res);
    }

  return TRUE;
}

static guint32
read_unaligned_uint32 (void *ptr)
{
  guint8 *p = ptr;
  guint32 val;

  val = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);

  return val;
}

static GVariant *
read_variant (gchar *_ptr, gsize len, gsize *size_out)
{
  guint32 size;
  guint8 *ptr = (guint8 *)_ptr;

  if (len < 4)
    return NULL;
  
  size = read_unaligned_uint32 (ptr);
  if (4 + size <= len)
    {
      GVariant *v, *v2;
      char *data;

      data = g_memdup (ptr + 4, size);
      v = g_variant_new_from_data (G_VARIANT_TYPE_VARIANT,
				   data, size, FALSE, g_free, data);

      v2 = g_variant_get_child_value (v, 0);
      g_variant_unref (v);
      
      *size_out = 4 + size;
      return v2;
    }

  return NULL;
}

GVariant *
dtools_variant_reader_next (DtoolsVariantReader *reader)
{
  if (reader->stream_format == -1)
    {
      while (reader->buffer->len <= 2)
	{
	  if (!read_some (reader))
	    return NULL;
	}

      if (reader->buffer->str[0] == '<' &&
	  reader->buffer->str[1] == 0)
	{
	  reader->stream_format = FORMAT_BINARY;
	  memmove (reader->buffer->str, reader->buffer->str + 2,
		   reader->buffer->len - 2);
	  g_string_set_size (reader->buffer,
			     reader->buffer->len - 2);
	}
      else
	reader->stream_format = FORMAT_TEXT;
    }

  if (reader->stream_format == FORMAT_BINARY)
    {
      guint32 size;
      GVariant *v;
      gsize v_size;

      /* If we already read a variant, process all existing full
	 variants before reading more. */
      if (reader->offset != 0)
	{
	  v = read_variant (reader->buffer->str + reader->offset,
			    reader->buffer->len - reader->offset,
			    &v_size);
	  if (v != NULL)
	    {
	      reader->offset += v_size;
	      return v;
	    }

	  /* Partial item at end, copy to begining */
	  memmove (reader->buffer->str,
		   reader->buffer->str + reader->offset,
		   reader->buffer->len - reader->offset);
	  g_string_set_size (reader->buffer,
			     reader->buffer->len - reader->offset);
	  reader->offset = 0;
	}

      /* Read at least one full variant */
      while (reader->buffer->len <= 4)
	{
	  if (!read_some (reader))
	    return NULL;
	}

      size = read_unaligned_uint32 (reader->buffer->str);
	  
      while (reader->buffer->len <= size)
	{
	  if (!read_some (reader))
	    return NULL;
	}

      v = read_variant (reader->buffer->str + reader->offset,
			reader->buffer->len - reader->offset,
			&v_size);
      g_assert (v != NULL);

      reader->offset = v_size;
      return v;
    }
  else
    {
      g_error ("text");
    }
    
  return NULL;
}
