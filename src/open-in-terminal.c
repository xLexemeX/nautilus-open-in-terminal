/* Nautilus extension: adds "Open in Terminal" to the main context menu.
 * Copyright (C) 2026 xLexemeX
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gio/gdesktopappinfo.h>
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

/* Builds the command for a terminal's desktop entry the way the Default
 * Terminal spec (xdg-terminal-exec) does: prefer the entry's "new-window"
 * action, and pass the folder through X-TerminalArgDir when it has one. */
static GStrv
build_command (const char *terminal,
               const char *path)
{
    g_autofree char *desktop_id = g_strconcat (terminal, ".desktop", NULL);
    g_autoptr (GDesktopAppInfo) info = g_desktop_app_info_new (desktop_id);
    g_autoptr (GKeyFile) keyfile = g_key_file_new ();
    g_autoptr (GStrvBuilder) builder = g_strv_builder_new ();
    g_auto (GStrv) actions = NULL;
    g_autofree char *exec = NULL;
    g_autofree char *dir_arg = NULL;
    g_auto (GStrv) argv = NULL;

    if (info == NULL ||
        !g_key_file_load_from_file (keyfile, g_desktop_app_info_get_filename (info), G_KEY_FILE_NONE, NULL))
        return NULL;

    actions = g_key_file_get_string_list (keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ACTIONS, NULL, NULL);
    if (actions != NULL && g_strv_contains ((const char * const *) actions, "new-window"))
        exec = g_key_file_get_string (keyfile, "Desktop Action new-window", G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
    if (exec == NULL)
        exec = g_key_file_get_string (keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
    if (exec == NULL || !g_shell_parse_argv (exec, NULL, &argv, NULL))
        return NULL;

    for (char **arg = argv; *arg != NULL; arg++)
    {
        /* Field codes don't apply when opening a folder. */
        if ((*arg)[0] == '%' && (*arg)[1] != '\0' && (*arg)[1] != '%' && (*arg)[2] == '\0')
            continue;
        g_strv_builder_add (builder, (*arg)[0] == '%' && (*arg)[1] == '%' ? *arg + 1 : *arg);
    }

    dir_arg = g_key_file_get_string (keyfile, G_KEY_FILE_DESKTOP_GROUP, "X-TerminalArgDir", NULL);
    if (dir_arg != NULL && g_str_has_suffix (dir_arg, "="))
    {
        g_strv_builder_take (builder, g_strconcat (dir_arg, path, NULL));
    }
    else if (dir_arg != NULL && dir_arg[0] != '\0')
    {
        g_strv_builder_add (builder, dir_arg);
        g_strv_builder_add (builder, path);
    }

    return g_strv_builder_end (builder);
}

static void
on_activate (NautilusMenuItem *item,
             OpenInTerminal   *self)
{
    const char *path = g_object_get_data (G_OBJECT (item), "path");
    g_autofree char *terminal = NULL;
    g_auto (GStrv) argv = NULL;
    g_autoptr (GError) error = NULL;

    if (self->settings != NULL)
        terminal = g_settings_get_string (self->settings, "terminal");

    argv = terminal != NULL ? build_command (terminal, path) : NULL;
    if (argv == NULL)
    {
        g_warning ("Open in Terminal: no installed terminal found for '%s'", terminal);
        return;
    }

    /* Terminals without X-TerminalArgDir start in the current directory. */
    if (!g_spawn_async (path, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error))
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
