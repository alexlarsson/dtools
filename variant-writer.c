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

#include "variant-writer.h"

struct DtoolsVariantWriter
{
  GObject parent_instance;
  int fd;
};
  
G_DEFINE_TYPE (DtoolsVariantWriter, dtools_variant_writer, G_TYPE_OBJECT)

static void
dtools_variant_writer_init (DtoolsVariantWriter *self)
{
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

  object_class->finalize     = dtools_variant_writer_finalize;
}

DtoolsVariantWriter *
dtools_variant_writer_new (int fd)
{
  return g_object_new (DTOOLS_TYPE_VARIANT_WRITER, NULL);
}

void
dtools_variant_writer_add (DtoolsVariantWriter *writer,
			   GVariant *value)
{
  char *str;
  size_t len, written;
  ssize_t res;
  GVariant *variant;

  variant = g_variant_new_variant (value);
  str = g_variant_print (variant, FALSE);
  g_variant_unref (variant);
  
  len = strlen (str);

  written = 0;
  while (len > written)
    {
      do
	res = write (writer->fd, str + written, len - written);
      while (res < 0 && errno == EINTR);
	
      if (res <= 0)
	{
	  g_warning ("Error writing variant\n");
	  break;
	}
      else
	written += res;
    }

  do
    res = write (writer->fd, "\n", 1);
  while (res < 0 && errno == EINTR);
  
  g_free (str);
}
