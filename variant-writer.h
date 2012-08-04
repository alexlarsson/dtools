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

#ifndef _DTOOLS_VARIANT_WRITER_H_
#define _DTOOLS_VARIANT_WRITER_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define DTOOLS_TYPE_VARIANT_WRITER (dtools_variant_writer_get_type ())
#define DTOOLS_VARIANT_WRITER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DTOOLS_TYPE_VARIANT_WRITER, DtoolsVariantWriter))
#define DTOOLS_VARIANT_WRITER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DTOOLS_TYPE_VARIANT_WRITER, DtoolsVariantWriterClass))
#define DTOOLS_IS_VARIANT_WRITER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DTOOLS_TYPE_VARIANT_WRITER))
#define DTOOLS_IS_VARIANT_WRITER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DTOOLS_TYPE_VARIANT_WRITER))
#define DTOOLS_VARIANT_WRITER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DTOOLS_TYPE_VARIANT_WRITER, DtoolsVariantWriterClass))

typedef struct
{
  GObjectClass parent_class;
} DtoolsVariantWriterClass;

typedef struct DtoolsVariantWriter DtoolsVariantWriter;

GType        dtools_variant_writer_get_type (void) G_GNUC_CONST;
DtoolsVariantWriter *dtools_variant_writer_new (int fd);
void dtools_variant_writer_add (DtoolsVariantWriter *writer,
				GVariant *value);

G_END_DECLS

#endif /* _DTOOLS_VARIANT_WRITER_H_ */
