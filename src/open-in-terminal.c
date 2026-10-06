/* Nautilus extension: adds "Open in Terminal" to the main context menu.
 * Copyright (C) 2026 xLexemeX
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gio/gio.h>
#include <nautilus-extension.h>

#define SCHEMA_ID "io.github.xlexemex.nautilus-open-in-terminal"

#define OPEN_TYPE_IN_TERMINAL (open_in_terminal_get_type ())
G_DECLARE_FINAL_TYPE (OpenInTerminal, open_in_terminal, OPEN, IN_TERMINAL, GObject)

struct _OpenInTerminal
{
    GObject parent_instance;
    GSettings *settings;
};

static void open_in_terminal_menu_provider_init (NautilusMenuProviderInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (OpenInTerminal, open_in_terminal, G_TYPE_OBJECT, G_TYPE_FLAG_FINAL,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (NAUTILUS_TYPE_MENU_PROVIDER,
                                                               open_in_terminal_menu_provider_init))

static GSettings *
load_settings (void)
{
    GSettingsSchemaSource *source = g_settings_schema_source_get_default ();
    g_autoptr (GSettingsSchema) schema = NULL;

    if (source != NULL)
        schema = g_settings_schema_source_lookup (source, SCHEMA_ID, TRUE);

    return schema != NULL ? g_settings_new_full (schema, NULL, NULL) : NULL;
}

static void
on_activate (NautilusMenuItem *item,
             OpenInTerminal   *self)
{
    const char *path = g_object_get_data (G_OBJECT (item), "path");
    g_autofree char *terminal = NULL;
    g_autoptr (GError) error = NULL;

    if (self->settings != NULL)
        terminal = g_settings_get_string (self->settings, "terminal");

    if (g_strcmp0 (terminal, "ptyxis") == 0)
    {
        const char *argv[] = { "ptyxis", "--new-window", "--working-directory", path, NULL };
        g_spawn_async (path, (char **) argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
    }
    else
    {
        const char *argv[] = { "gnome-terminal", "--working-directory", path, NULL };
        g_spawn_async (path, (char **) argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
    }

    if (error != NULL)
        g_warning ("Open in Terminal: %s", error->message);
}

static GList *
make_item (OpenInTerminal   *self,
           const char       *name,
           NautilusFileInfo *file)
{
    g_autoptr (GFile) location = nautilus_file_info_get_location (file);
    g_autofree char *path = location != NULL ? g_file_get_path (location) : NULL;
    NautilusMenuItem *item;

    if (path == NULL)
        return NULL;

    item = nautilus_menu_item_new (name, "Open in Terminal", NULL, NULL);
    g_object_set_data_full (G_OBJECT (item), "path", g_steal_pointer (&path), g_free);
    g_signal_connect_object (item, "activate", G_CALLBACK (on_activate), self, 0);

    return g_list_append (NULL, item);
}

static GList *
get_file_items (NautilusMenuProvider *provider,
                GList                *files)
{
    if (files == NULL || files->next != NULL || !nautilus_file_info_is_directory (files->data))
        return NULL;

    return make_item (OPEN_IN_TERMINAL (provider), "OpenInTerminal::Folder", files->data);
}

static GList *
get_background_items (NautilusMenuProvider *provider,
                      NautilusFileInfo     *current_folder)
{
    return make_item (OPEN_IN_TERMINAL (provider), "OpenInTerminal::Background", current_folder);
}

static void
open_in_terminal_menu_provider_init (NautilusMenuProviderInterface *iface)
{
    iface->get_file_items = get_file_items;
    iface->get_background_items = get_background_items;
}

static void
open_in_terminal_finalize (GObject *object)
{
    g_clear_object (&OPEN_IN_TERMINAL (object)->settings);
    G_OBJECT_CLASS (open_in_terminal_parent_class)->finalize (object);
}

static void
open_in_terminal_init (OpenInTerminal *self)
{
    self->settings = load_settings ();
}

static void
open_in_terminal_class_init (OpenInTerminalClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = open_in_terminal_finalize;
}

static void
open_in_terminal_class_finalize (G_GNUC_UNUSED OpenInTerminalClass *klass)
{
}

static GType types[1];

void
nautilus_module_initialize (GTypeModule *module)
{
    open_in_terminal_register_type (module);
    types[0] = OPEN_TYPE_IN_TERMINAL;
}

void
nautilus_module_shutdown (void)
{
}

void
nautilus_module_list_types (const GType **out_types,
                            int          *num_types)
{
    *out_types = types;
    *num_types = G_N_ELEMENTS (types);
}
