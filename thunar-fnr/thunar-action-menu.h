/*-
 * Copyright (C) 2015  Peter de Ridder <peter@xfce.org>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __THUNAR_ACTION_MENU_H__
#define __THUNAR_ACTION_MENU_H__

#include <gtk/gtk.h>
#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _ThunarActionMenuClass ThunarActionMenuClass;
typedef struct _ThunarActionMenu      ThunarActionMenu;

#define THUNAR_TYPE_ACTION_MENU             (thunar_action_menu_get_type ())
#define THUNAR_ACTION_MENU(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), THUNAR_TYPE_ACTION_MENU, ThunarActionMenu))
#define THUNAR_ACTION_MENU_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), THUNAR_TYPE_ACTION_MENU, ThunarActionMenuClass))
#define THUNAR_IS_ACTION_MENU(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), THUNAR_TYPE_ACTION_MENU))
#define THUNAR_IS_ACTION_MENU_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), THUNAR_TYPE_ACTION_MENU))
#define THUNAR_ACTION_MENU_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), THUNAR_TYPE_ACTION_MENU, ThunarActionMenuClass))

GType      thunar_action_menu_get_type      (void) G_GNUC_CONST G_GNUC_INTERNAL;
void       thunar_action_menu_register_type (ThunarxProviderPlugin *) G_GNUC_INTERNAL;

GtkAction *thunar_action_menu_new           (const gchar *name,
											const gchar *label,
											const gchar *tooltip,
											const gchar *stock_id,
											GList *menu) G_GNUC_MALLOC G_GNUC_INTERNAL;
G_END_DECLS;

#endif /* !__THUNAR_ACTION_MENU_H__ */
