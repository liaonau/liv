#include "conf.h"
#include "util.h"

#include "fileL.h"
#include "imageL.h"
#include "gridL.h"

#include <math.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <glib/gprintf.h>

gboolean luaH_init(void)
{
    L = luaL_newstate();
    if (L == NULL)
        return FALSE;
    luaL_openlibs(L);
    luaopen_fileL(L, LIB_FILEL);
    luaopen_imageL(L, LIB_IMAGEL);
    luaopen_gridL(L, LIB_GRIDL);
    return TRUE;
}

gboolean luaH_loadrc(gchar* confpath)
{
    const gchar* const *config_dirs = g_get_system_config_dirs();
    GPtrArray          *ptr_paths   = g_ptr_array_new();
    if (confpath != NULL)
        g_ptr_array_add(ptr_paths, confpath);
    g_ptr_array_add(ptr_paths, g_build_filename(g_get_user_config_dir(), APPNAME, "/rc.lua", NULL));
    for(; *config_dirs; config_dirs++)
        g_ptr_array_add(ptr_paths, g_build_filename(*config_dirs, APPNAME, "/rc.lua", NULL));
    gchar** paths = (gchar**)g_ptr_array_free(ptr_paths, FALSE);

    gchar** p = paths;
    for (gint i = 0; p[i] && p ; i++)
    {
        if (luaL_dofile(L, p[i]) == 0)
            return TRUE;
        else
            warn("«%s» doesn't exist", p[i]);
    }
    return FALSE;
}

gint main(gint argc, gchar **argv)
{
    setlocale(LC_ALL, "");
    if (argc < 2)
        fatal("nothing to display");

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

    swindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), swindow);
    g_object_ref(swindow);

    if (!luaH_init())
        fatal("can't init Lua");

    if (!luaH_loadrc(rcfile))
        fatal("can't find config file anywhere");

    lua_getglobal(L, "init");
    for (gint i = 1; i < argc; i++)
        lua_pushstring(L, argv[i]);
    lua_pcall(L, argc - 1, 0, 0);

    /*g_signal_connect(window, "size-allocate",   G_CALLBACK(cb_size),       NULL);*/
    /*g_signal_connect(window, "key-press-event", G_CALLBACK(cb_key),        NULL);*/
    g_signal_connect(window, "destroy",         G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all((GtkWidget*)window);
    gtk_main();

    return 0;
}
