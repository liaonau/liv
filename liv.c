/*
 * Copyright © 2017 Roman Leonov <rliaonau@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "conf.h"
#include "util.h"
#include "task.h"
#include "inlined.h"

#include "appL.h"
#include "imageL.h"
#include "gridL.h"
#include "frameL.h"

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

static lua_State* init_lua_State(void)
{
    lua_State* L = luaL_newstate();
    if (L == NULL)
        return NULL;
    luaL_openlibs(L);
    luaopen_appL(L);
    luaopen_frameL(L);
    luaopen_imageL(L);
    luaopen_gridL(L);
    return L;
}
static int luaH_traceback(lua_State *L)
{
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_replace(L, -2);
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    return 1;
}
static void luaH_pcall(lua_State *L, int nargs, int nresults)
{
    lua_pushcfunction(L, luaH_traceback);
    lua_insert(L, - nargs - 2);
    int error_func_pos = lua_gettop(L) - nargs - 1;
    if (lua_pcall(L, nargs, nresults, -nargs - 2))
    {
        warn("%s", lua_tostring(L, -1));
        lua_pop(L, 2);
        return;
    }
    lua_remove(L, error_func_pos);
}
static gboolean luaH_loadrc(lua_State* L, gchar* confpath)
{
    const gchar* const *config_dirs = g_get_system_config_dirs();
    GPtrArray          *ptr_paths   = g_ptr_array_new();
    if (confpath != NULL)
        g_ptr_array_add(ptr_paths, confpath);
    g_ptr_array_add(ptr_paths, "./rc.lua");
    g_ptr_array_add(ptr_paths, g_build_filename(g_get_user_config_dir(), APPNAME, "/rc.lua", NULL));
    for(; *config_dirs; config_dirs++)
        g_ptr_array_add(ptr_paths, g_build_filename(*config_dirs, APPNAME, "/rc.lua", NULL));
    gchar** paths = (gchar**)g_ptr_array_free(ptr_paths, FALSE);

    gchar** p = paths;
    for (gint i = 0; p[i] && p ; i++)
    {
        /*info("loadind «%s»", p[i]);*/
        if (!g_file_test(p[i], G_FILE_TEST_EXISTS))
            warn("«%s» doesn't exist", p[i]);
        else
            if (luaL_loadfile(L, p[i]) || lua_pcall(L, 0, 0, 0))
            {
                const char* msg = lua_tostring(L, -1);
                if (msg == NULL)
                    msg = "(error with no message)";
                warn("can't parse «%s».\n %s", p[i], msg);
                lua_pop(L, 1);
            }
            else
                return TRUE;
    }
    return FALSE;
}

#define LUA_IF_CB_FUNCTION(data, field, func, ev) \
{                                                 \
    lua_State* L = (lua_State*)data;              \
    gint top = lua_gettop(L);                     \
    lua_getglobal(L, "callbacks");                \
    if (lua_istable(L, -1))                       \
    {                                             \
        lua_getfield(L, -1, field);               \
        if (lua_isfunction(L, -1))                \
        {                                         \
           func(L, ev);                           \
        }                                         \
    }                                             \
    lua_settop(L, top);                           \
}

static void cb_size_func(lua_State* L, GdkRectangle* rect)
{
    lua_pushnumber(L, rect->x);
    lua_pushnumber(L, rect->y);
    lua_pushnumber(L, rect->width);
    lua_pushnumber(L, rect->height);
    luaH_pcall(L, 4, 0);
}
static void cb_size(GtkWidget* widget, GdkRectangle* rect, gpointer data)
{
    LUA_IF_CB_FUNCTION(data, "size", cb_size_func, rect);
}

#define MODKEY(key, name)                \
    if (state & GDK_##key##_MASK)        \
    {                                    \
        lua_pushstring(L, name);         \
        gint idx = luaL_getn(L, -2) + 1; \
        lua_rawseti(L, -2, idx);         \
    }
static void cb_key_func(lua_State* L, GdkEventKey* ev)
{
    guint state = ev->state;
    lua_newtable(L);
    if (state & GDK_MODIFIER_MASK)
    {
        MODKEY(SHIFT,   "Shift");
        MODKEY(CONTROL, "Control");
        MODKEY(MOD1,    "Mod1");
    }
    guint val = ev->keyval;
    gdk_keymap_translate_keyboard_state(
            gdk_keymap_get_default(),
            ev->hardware_keycode,
            ev->state & !GDK_SHIFT_MASK,
            0, //default group
            &val,
            NULL, NULL, NULL);
    lua_pushstring(L, g_strdup(gdk_keyval_name(val)));
    lua_pushnumber(L, val);
    luaH_pcall(L, 3, 0);
}
static void cb_key(GtkWidget* widget, GdkEventKey* ev, gpointer data)
{
    if (ev->type != GDK_KEY_PRESS || ev->is_modifier == 1)
        return;
    LUA_IF_CB_FUNCTION(data, "keypress", cb_key_func, ev);
}

static void cb_button_func(lua_State* L, GdkEventButton* ev)
{
    guint state = ev->state;
    lua_newtable(L);
    if (state & GDK_MODIFIER_MASK)
    {
        MODKEY(SHIFT,   "Shift");
        MODKEY(CONTROL, "Control");
        MODKEY(MOD1,    "Mod1");
    }
    lua_pushnumber(L, ev->button);
    lua_pushnumber(L, ev->x);
    lua_pushnumber(L, ev->y);
    luaH_pcall(L, 4, 0);
}
static void cb_button(GtkWidget* widget, GdkEventButton* ev, gpointer data)
{
    if (ev->type != GDK_BUTTON_PRESS)
        return;
    LUA_IF_CB_FUNCTION(data, "button", cb_button_func, ev);
}

gint main(gint argc, gchar **argv)
{
    setlocale(LC_ALL, "");

    gtk_init(&argc, &argv);

    gchar* rcfile = NULL;
    GOptionContext      *context;
    const GOptionEntry  entries[] =
    {
        { "rc", 'c', 0, G_OPTION_ARG_STRING, &rcfile, "rc.lua config file to use", NULL },
        { NULL,  0,  0, 0,                   NULL,     NULL,                       NULL },
    };
    context = g_option_context_new("[FILES]");
    g_option_context_add_main_entries(context, entries, NULL);
    GError* err = NULL;
    if (!g_option_context_parse(context, &argc, &argv, &err))
        fatal("%s\n", err->message);
    g_option_context_free(context);

    window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), APPNAME);
    gtk_widget_set_name((GtkWidget*)window, "window");

    GtkBox* mainbox;
    mainbox   = (GtkBox*)gtk_box_new(GTK_ORIENTATION_VERTICAL,   0);
    statusbox = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    scroll    = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);

    displayref = LUA_REFNIL;

    gtk_widget_set_hexpand((GtkWidget*)scroll, TRUE);
    gtk_widget_set_vexpand((GtkWidget*)scroll, TRUE);
    gtk_scrolled_window_set_policy(scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_width(scroll, TRUE);

    gtk_widget_set_name((GtkWidget*)scroll,    "content");
    gtk_widget_set_name((GtkWidget*)statusbox, "status");

    gtk_box_pack_start(mainbox, (GtkWidget*)scroll,    TRUE,  TRUE,  0);
    gtk_box_pack_end(  mainbox, (GtkWidget*)statusbox, FALSE, FALSE, 0);

#define NEW_LABEL(label, align)                                  \
    {                                                            \
        label = (GtkLabel*)gtk_label_new("");                    \
        gtk_label_set_xalign((label), (align));                  \
        gtk_label_set_ellipsize((label), PANGO_ELLIPSIZE_START); \
        gtk_label_set_line_wrap((label), FALSE);                 \
        gtk_widget_set_name((GtkWidget*)(label), #label);        \
    }
    NEW_LABEL(status_left,  0);
    NEW_LABEL(status_right, 1);

    gtk_box_pack_start(statusbox, (GtkWidget*)status_left,  TRUE,  TRUE,  0);
    gtk_box_pack_end(  statusbox, (GtkWidget*)status_right, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), (GtkWidget*)mainbox);

    if (!init_resources())
        fatal("can't init inlined resources");

    lua_State* L = init_lua_State();
    if (!L)
        fatal("can't init Lua");

    if (!luaH_loadrc(L, rcfile))
        fatal("can't find valid config file anywhere");

    lua_getglobal(L, "init");
    lua_checkstack(L, argc);
    for (gint i = 1; i < argc; i++)
        lua_pushstring(L, argv[i]);
    luaH_pcall(L, argc - 1, 0);

    g_signal_connect(window, "destroy",            G_CALLBACK(gtk_main_quit), (gpointer)L);
    g_signal_connect(window, "key-press-event",    G_CALLBACK(cb_key),        (gpointer)L);
    g_signal_connect(window, "size-allocate",      G_CALLBACK(cb_size),       (gpointer)L);
    g_signal_connect(window, "button-press-event", G_CALLBACK(cb_button),     (gpointer)L);

    gtk_widget_show_all((GtkWidget*)window);
    gtk_main();

    return 0;
}
