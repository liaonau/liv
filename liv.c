#include "conf.h"
#include "util.h"

#include "imageL.h"
#include "gridL.h"
#include "scrollL.h"
#include "appL.h"

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

static gboolean luaH_init(void)
{
    L = luaL_newstate();
    if (L == NULL)
        return FALSE;
    luaL_openlibs(L);
    luaopen_imageL(L, LIB_IMAGEL);
    luaopen_gridL(L, LIB_GRIDL);
    luaopen_scrollL(L, LIB_SCROLLL);
    luaopen_appL(L, LIB_APPL);
    return TRUE;
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

static gboolean luaH_loadrc(gchar* confpath)
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
        if (!g_file_test(p[i], G_FILE_TEST_EXISTS))
        {
            warn("«%s» doesn't exist", p[i]);
            continue;
        }
        /*if (luaL_dofile(L, p[i]) == 0)*/
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

static void cb_size(GtkWidget *widget, GdkRectangle *rect, gpointer data)
{
    gint top = lua_gettop(L);
    lua_getglobal(L, "callbacks");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "size");
        if (lua_isfunction(L, -1))
        {
            lua_pushstring(L, (gchar*)data);
            lua_pushnumber(L, rect->x);
            lua_pushnumber(L, rect->y);
            lua_pushnumber(L, rect->width);
            lua_pushnumber(L, rect->height);
            luaH_pcall(L, 5, 0);
        }
    }
    lua_settop(L, top);
}

static void cb_key(GtkWidget *widget, GdkEventKey *ev, gpointer user_data)
{
    if (ev->type != GDK_KEY_PRESS || ev->is_modifier == 1)
        return;
    gint top = lua_gettop(L);
    lua_getglobal(L, "callbacks");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "keypress");
        if (lua_isfunction(L, -1))
        {
            guint state = ev->state;
            lua_newtable(L);
            if (state & GDK_MODIFIER_MASK)
            {
                gint i = 1;
            #define MODKEY(key, name)         \
                if (state & GDK_##key##_MASK) \
                {                             \
                    lua_pushstring(L, name);  \
                    lua_rawseti(L, -2, i++);  \
                }
                /*MODKEY(LOCK,    "Lock");*/
                MODKEY(SHIFT,   "Shift");
                MODKEY(CONTROL, "Control");
                MODKEY(MOD1,    "Mod1");
                /*MODKEY(MOD2,    "Mod2");*/
                /*MODKEY(MOD3,    "Mod3");*/
                /*MODKEY(MOD4,    "Mod4");*/
                /*MODKEY(MOD5,    "Mod5");*/
            #undef MODKEY
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
    }
    lua_settop(L, top);
}

gint main(gint argc, gchar **argv)
{
    setlocale(LC_ALL, "");
    if (argc < 2)
    {
        info("nothing to display");
        exit(0);
    }

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

    mainbox   = (GtkBox*)gtk_box_new(GTK_ORIENTATION_VERTICAL,   0);
    content   = (GtkBox*)gtk_box_new(GTK_ORIENTATION_VERTICAL,   0);
    statusbox = (GtkBox*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name((GtkWidget*)content,   "content");
    gtk_widget_set_name((GtkWidget*)statusbox, "status");
    gtk_box_pack_start(mainbox, (GtkWidget*)content,   TRUE,  TRUE,  0);
    gtk_box_pack_end(  mainbox, (GtkWidget*)statusbox, FALSE, FALSE, 0);

    status_left  = (GtkLabel*)gtk_label_new("");
    status_right = (GtkLabel*)gtk_label_new("");
    gtk_label_set_xalign(status_left,  0);
    gtk_label_set_xalign(status_right, 1);
    gtk_label_set_line_wrap(status_left,  TRUE);
    gtk_label_set_line_wrap(status_right, TRUE);
    gtk_widget_set_name((GtkWidget*)status_left,  "status_left");
    gtk_widget_set_name((GtkWidget*)status_right, "status_right");
    gtk_box_pack_start(statusbox, (GtkWidget*)status_left,  TRUE,  TRUE,  0);
    gtk_box_pack_end(  statusbox, (GtkWidget*)status_right, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), (GtkWidget*)mainbox);

    if (!luaH_init())
        fatal("can't init Lua");

    if (!luaH_loadrc(rcfile))
        fatal("can't find valid config file anywhere");

    lua_getglobal(L, "init");
    lua_checkstack(L, argc);
    for (gint i = 1; i < argc; i++)
        lua_pushstring(L, argv[i]);
    luaH_pcall(L, argc - 1, 0);

    g_signal_connect(window,  "destroy",         G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window,  "key-press-event", G_CALLBACK(cb_key),        NULL);
    g_signal_connect(window,  "size-allocate",   G_CALLBACK(cb_size),       "window");
    g_signal_connect(content, "size-allocate",   G_CALLBACK(cb_size),       "content");

    gtk_widget_show_all((GtkWidget*)window);
    gtk_main();

    return 0;
}

