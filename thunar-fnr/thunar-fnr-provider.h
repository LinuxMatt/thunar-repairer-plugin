/* vi:set et ai sw=4 ts=4: */
/*-
 * Copyright (c) 2014 Matt Thirtytwo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __FNR_PROVIDER_H__
#define __FNR_PROVIDER_H__

#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _FnrProviderClass FnrProviderClass;
typedef struct _FnrProvider      FnrProvider;

#define FNR_TYPE_PROVIDER            (fnr_provider_get_type ())
#define FNR_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FNR_TYPE_PROVIDER, FnrProvider))
#define FNR_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FNR_TYPE_PROVIDER, FnrProviderClass))
#define FNR_IS_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FNR_TYPE_PROVIDER))
#define FNR_IS_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FNR_TYPE_PROVIDER))
#define FNR_PROVIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FNR_TYPE_PROVIDER, FnrProviderClass))

GType fnr_provider_get_type      (void) G_GNUC_CONST;
void  fnr_provider_register_type (ThunarxProviderPlugin *plugin);

G_END_DECLS;

#endif /* !__FNR_PROVIDER_H__ */
