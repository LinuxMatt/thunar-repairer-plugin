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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <glib/gi18n.h>
#include <thunar-fnr/thunar-fnr-provider.h>
#include <thunar-fnr/thunar-action-menu.h>

static void   fnr_provider_menu_provider_init (ThunarxMenuProviderIface *iface);
static void   fnr_provider_finalize           (GObject *object);
static GList *fnr_provider_get_file_actions   (ThunarxMenuProvider *menu_provider, GtkWidget *window, GList *files);

struct _FnrProviderClass
{
    GObjectClass __parent__;
};

struct _FnrProvider
{
    GObject __parent__;
    gchar   *child_watch_path;
    gint    child_watch_id;
};

// from http://www.microsoft.com/globaldev/reference/wincp.mspx
// Code Pages Supported by Windows
static const char* encoding_list[] = {
    "CP1252",  // Latin I  (default encoding)
    "CP1250",  // Central Europe
    "CP1251",  // Cyrillic
    "CP1253",  // Greek
    "CP1254",  // Turkish
    "CP1255",  // Hebrew
    "CP1256",  // Arabic
    "CP1257",  // Baltic
    "CP1258",  // Vietnam
    "CP874",   // Thai
    "CP932",   // Japanese Shift-JIS
    "CP936",   // Simplified Chinese GBK
    "CP949",   // Korean
    "CP950",   // Traditional Chinese Big5
    NULL
};

struct encoding_item {
    const char* locale;
    const char* encoding;
};

static const struct encoding_item default_encoding_list[] = {
    { "ar",  "CP1256" },
    { "az",  "CP1251" },
    { "az",  "CP1254" },
    { "be",  "CP1251" },
    { "bg",  "CP1251" },
    { "cs",  "CP1250" },
    { "cy",  "CP1253" },
    { "el",  "CP1253" },
    { "et",  "CP1257" },
    { "fa",  "CP1256" },
    { "he",  "CP1255" },
    { "hr",  "CP1250" },
    { "hu",  "CP1250" },
    { "ja",  "CP932"  },
    { "kk",  "CP1251" },
    { "ko",  "CP949"  },
    { "ky",  "CP1251" },
    { "lt",  "CP1257" },
    { "lv",  "CP1257" },
    { "mk",  "CP1251" },
    { "mn",  "CP1251" },
    { "pl",  "CP1250" },
    { "ro",  "CP1250" },
    { "ru",  "CP1251" },
    { "sk",  "CP1250" },
    { "sl",  "CP1250" },
    { "sq",  "CP1250" },
    { "sr",  "CP1250" },
    { "sr",  "CP1251" },
    { "th",  "CP874"  },
    { "tr",  "CP1254" },
    { "tt",  "CP1251" },
    { "uk",  "CP1251" },
    { "ur",  "CP1256" },
    { "uz",  "CP1251" },
    { "uz",  "CP1254" },
    { "vi",  "CP1258" },
    { "zh",  "CP936"  },
    { "zh",  "CP950"  },
    { NULL,  NULL     }
};

THUNARX_DEFINE_TYPE_WITH_CODE (FnrProvider, fnr_provider, G_TYPE_OBJECT,
                        THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_MENU_PROVIDER,
                        fnr_provider_menu_provider_init));

static void fnr_provider_class_init (FnrProviderClass *klass)
{
    GObjectClass *gobject_class;
    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = fnr_provider_finalize;
}

static void fnr_provider_menu_provider_init (ThunarxMenuProviderIface *iface)
{
    iface->get_file_actions = fnr_provider_get_file_actions;
}

static void fnr_provider_init (FnrProvider *fnr_provider)
{
}

static void fnr_provider_finalize (GObject *object)
{
    G_OBJECT_CLASS (fnr_provider_parent_class)->finalize (object);
}

static gboolean need_repair_dialog(GList* files)
{
    char* name;
    gboolean res;

    while (files != NULL) {
        if (thunarx_file_info_is_directory (files->data))
            return TRUE;

        name = thunarx_file_info_get_name(files->data);
        res = g_utf8_validate(name, -1, NULL);
        g_free(name);
        if (!res) return TRUE;
        files = g_list_next(files);
    }
    return FALSE;
}

static char* get_filename_without_mnemonic(const char* filename)
{
    GString *str = g_string_new(NULL);

    while (*filename != '\0') {
        if (*filename == '_')
            g_string_append(str, "__");
        else
            g_string_append_c(str, *filename);
        filename++;
    }
    return g_string_free(str, FALSE);
}

static void show_error_message(GtkWidget* parent, const char* filename, GError* error)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(parent),
                GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
            _("<span size=\"larger\" weight=\"bold\">There was an error renaming the file to \"%s\"</span>"),
                filename);

    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),
                                                "%s", error->message);
    gtk_dialog_run(GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);
}

static void on_rename_menu_item_activated(GtkWidget *item, gpointer *data)
{
    gboolean res;
    gchar* old_name;
    gchar* new_name;
    GFile* file;
    GFile* new_file;
    GFile* parent;
    GtkWidget* window;
    GError* error = NULL;

    new_name = g_object_get_data(G_OBJECT(item), "Repairer::new_name");
    file = g_object_get_data(G_OBJECT(item), "Repairer::file");
    window = g_object_get_data(G_OBJECT(item), "Repairer::window");
    parent = g_file_get_parent(file);
    new_file = g_file_get_child(parent, new_name);
    old_name = g_file_get_parse_name (file);
    g_free(old_name);

    res = g_file_move(file, new_file, G_FILE_COPY_NOFOLLOW_SYMLINKS, NULL, NULL, NULL, &error);
    if (!res) {
        if (error->code == G_IO_ERROR_EXISTS) {
            GtkWidget *dialog;

            dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(window),
                    GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                    GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
                    _("<span size=\"larger\" weight=\"bold\">A file named \"%s\" already exists.</span>"),
                    new_name);

            gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),
                    _("If you want to rename the selected file, " "please move or rename \"%s\" first."),
                    new_name);

            gtk_dialog_run(GTK_DIALOG (dialog));
            gtk_widget_destroy(dialog);
        } else {
            show_error_message(window, new_name, error);
        }
        g_error_free(error);
    }
    g_object_unref(parent);
    g_object_unref(new_file);
}

static GtkWidget* rename_menu_item_new(const char* name, GFile* file, int menu_index, GtkWidget* window, gboolean is_submenu)
{
    gchar id[128];
    gchar* filename;
    gchar* label;
    gchar* tooltip;
    GtkWidget* item;

    filename = get_filename_without_mnemonic(name);

    g_snprintf(id, sizeof(id), "Repairer::rename_as_%d", menu_index);
    if (is_submenu)
        label = g_strdup(filename);
    else
        label = g_strdup_printf(_("Re_name as \"%s\""), filename);

    tooltip = g_strdup_printf(_("Rename as \"%s\"."), name);

    g_object_ref(file);

    item  = g_object_new (GTK_TYPE_ACTION, "name", id, "label", label, "tooltip", tooltip, NULL);

    g_object_set_data(G_OBJECT(item), "Repairer::window", window);
    g_object_set_data_full(G_OBJECT(item), "Repairer::file", file, g_object_unref);
    g_object_set_data_full(G_OBJECT(item), "Repairer::new_name", g_strdup(name), g_free);
    g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_rename_menu_item_activated), NULL);

    g_free(filename);
    g_free(label);
    g_free(tooltip);

    return item;
}

static GList* append_uri_item(GList* menu, const char* name, const char* new_name, GFile* file, GtkWidget* window)
{
    GtkWidget* item;
    int menu_index;

    if (strcmp(name, new_name) != 0) {
        menu_index = g_list_length(menu);
        item = rename_menu_item_new(new_name, file, menu_index, window, FALSE);
        menu = g_list_append(menu, item);
    }
    return menu;
}

static GList* append_unicode_nfc_item(GList* menu, const char* name, GFile* file, GtkWidget* window)
{
    GtkWidget* item;
    char* nfc;
    int menu_index;

    // test for MacOSX filename which is NFD
    nfc = g_utf8_normalize(name, -1, G_NORMALIZE_NFC);
    if (nfc != NULL) {
        if (strcmp(name, nfc) != 0) {
            menu_index = g_list_length(menu);
            item = rename_menu_item_new(nfc, file, menu_index, window, FALSE);
            menu = g_list_append(menu, item);
        }
        g_free(nfc);
    }
    return menu;
}

static GList* append_default_encoding_items(GList* menu, const char* name, GFile* file, GtkWidget* window)
{
    char* locale;
    const struct encoding_item* e;
    GtkWidget* item;
    int menu_index;

    locale = setlocale(LC_CTYPE, NULL);
    if (locale == NULL) {
        return menu;
    }

    menu_index = g_list_length(menu);
    for (e = default_encoding_list; e->locale != NULL; e++) {
        size_t len = strlen(e->locale);
        if (strncmp(e->locale, locale, len) == 0) {
            gchar* new_name;
            new_name = g_convert(name, -1, "UTF-8", e->encoding, NULL, NULL, NULL);
            if (new_name == NULL)
                continue;

            if (strcmp(name, new_name) != 0) {
                item = rename_menu_item_new(new_name, file, menu_index, window, FALSE);
                menu = g_list_append(menu, item);
                menu_index++;
            }
            g_free(new_name);
        }
    }
    return menu;
}

static GList* append_other_encoding_items(GList* menu, const char* name, GFile* file, GtkWidget* window)
{
    GtkWidget* item;
    GTree* new_name_table;
    gchar* new_name;
    int i;
    int menu_index;
    gpointer have_item;

    menu_index = g_list_length(menu);
    new_name_table = g_tree_new_full((GCompareDataFunc)strcmp, NULL, g_free, NULL);
    for (i = 0; encoding_list[i] != NULL; i++) {
        new_name = g_convert(name, -1, "UTF-8", encoding_list[i], NULL, NULL, NULL);
        if (new_name == NULL)
            continue;

        if (strcmp(name, new_name) == 0) {
            g_free(new_name);
            continue;
        }
        have_item = g_tree_lookup(new_name_table, new_name);
        if (have_item == NULL) {
            item = rename_menu_item_new(new_name, file, menu_index, window, TRUE);
            menu = g_list_append(menu, item);
            g_tree_insert(new_name_table, new_name, new_name);
            menu_index++;
        } else {
            g_free(new_name);
        }
    }
    g_tree_destroy(new_name_table);

    return menu;
}

static GList* fnr_append_menu_items(ThunarxMenuProvider *menu, GtkWidget *window, GList *files)
{
    GList *actions = NULL;
    gboolean is_native;
    gchar* name;
    gchar* unescaped;
    gchar* reconverted;
    GFile *file = NULL;

    if(files==NULL) {
        return NULL;
    }
    if (files->next != NULL) {
        return NULL;
    }
    /*
    if (thunarx_file_info_is_directory (files->data)) {
       return actions;
    }
    */

    /* get the location of the file */
    file = thunarx_file_info_get_location (files->data);
    if(file==NULL) {
        return NULL;
    }
    /* unable to handle non-local files */
    if (G_UNLIKELY (!g_file_has_uri_scheme (file, "file"))) {
        g_object_unref (file);
        return NULL;
    }

    is_native = g_file_is_native(file);
    if (!is_native) {
        return NULL;
    }
    name = g_file_get_basename(file);
    if (name == NULL) {
        return NULL;
    }
    unescaped = g_uri_unescape_string(name, NULL);
    if (unescaped != NULL) {
        if (g_utf8_validate(unescaped, -1, NULL)) {
            actions = append_uri_item(actions, name, unescaped, file, window);
            actions = append_unicode_nfc_item(actions, unescaped, file, window);
        }
        g_free(name);
        name = unescaped;
    }
    if (g_utf8_validate(name, -1, NULL)) {
        reconverted = g_convert(name, -1, "CP1252", "UTF-8", NULL, NULL, NULL);
        if (reconverted != NULL) {
            actions = append_default_encoding_items(actions, reconverted, file, window);
            actions = append_other_encoding_items(actions, reconverted, file, window);
            g_free(reconverted);
        }
    } else {
        actions = append_default_encoding_items(actions, name, file, window);
        actions = append_other_encoding_items(actions, name, file, window);
    }

    g_free(name);
    g_object_unref(file);

    return actions;
}

static GList* fnr_provider_get_file_actions (ThunarxMenuProvider *menu_provider,
                                                GtkWidget *window, GList *files)
{
    GList* menu_items = NULL;
    GList *actions = NULL;
    GtkAction *menu = NULL;

    menu_items = fnr_append_menu_items(menu_provider, window, files);
    if(menu_items) {
        menu = thunar_action_menu_new ("Menu_Name","Repair encoding", "Menu_Tooltip", "background", menu_items);
        actions = g_list_append (actions, menu);
    }
    if (need_repair_dialog(files)) {
        // TODO
        // g_message("it needs repair dialog");
    }
    return actions;
}

