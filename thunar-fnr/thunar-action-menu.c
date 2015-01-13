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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <thunarx/thunarx.h>
#include <libxfce4util/libxfce4util.h>
#include "thunar-action-menu.h"

struct _ThunarActionMenuClass
{
	GtkActionClass __parent__;
};

struct _ThunarActionMenu
{
	GtkAction __parent__;
	GList *menu;
};

static GtkWidget *thunar_action_menu_create_menu_item (GtkAction *action);
static GtkWidget *thunar_action_menu_create_menu (GtkAction *action);
static void thunar_action_menu_finalize (GObject*);

THUNARX_DEFINE_TYPE (ThunarActionMenu, thunar_action_menu, GTK_TYPE_ACTION)

static void thunar_action_menu_class_init (ThunarActionMenuClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GtkActionClass *gtkaction_class = GTK_ACTION_CLASS (klass);

	gobject_class->finalize = thunar_action_menu_finalize;

	gtkaction_class->create_menu_item = thunar_action_menu_create_menu_item;
	gtkaction_class->create_menu = thunar_action_menu_create_menu;
}

static void thunar_action_menu_init (ThunarActionMenu *self)
{
	self->menu = NULL;
}

GtkAction *thunar_action_menu_new (const gchar *name,
									const gchar *label,
									const gchar *tooltip,
									const gchar *stock_id,
									GList *menu)
{
	GtkAction *action;

	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(label, NULL);

	action = g_object_new (THUNAR_TYPE_ACTION_MENU,
							"name", name,
							"label", label,
							"tooltip", tooltip,
							"stock-id", stock_id,
							NULL);
	THUNAR_ACTION_MENU (action)->menu = menu;
	return action;
}

static void thunar_action_menu_finalize (GObject *object)
{
	GList *iter;

	for (iter = THUNAR_ACTION_MENU (object)->menu; iter; iter = iter->next)
	{
		g_object_unref (iter->data);
	}
	g_list_free (THUNAR_ACTION_MENU (object)->menu);
	THUNAR_ACTION_MENU (object)->menu = NULL;

	G_OBJECT_CLASS (thunar_action_menu_parent_class)->finalize (object);
}

static GtkWidget * thunar_action_menu_create_menu_item (GtkAction *action)
{
	GtkWidget *item;
	GtkWidget *menu;

	item = GTK_ACTION_CLASS(thunar_action_menu_parent_class)->create_menu_item (action);

	menu = gtk_action_create_menu (action);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);

	return item;
}

static GtkWidget *thunar_action_menu_create_menu (GtkAction *action)
{
	GtkWidget *menu;
	GList *iter;

	menu = gtk_menu_new ();

	for (iter = THUNAR_ACTION_MENU (action)->menu; iter; iter = iter->next)
	{
		GtkAction *subaction;
		GtkWidget *subitem;

		subaction = GTK_ACTION (iter->data);
		subitem = gtk_action_create_menu_item (subaction);

		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}

	return menu;
}

