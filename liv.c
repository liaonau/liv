#include "conf.h"
#include "opts.h"
#include "util.h"

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

GtkWindow* window;
GtkImage* image;
lua_State *L;

void read_image(const gchar* filename)
{
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if (pixbuf != NULL)
    {
        warn("loading %s", filename);
        image_t* image  = g_malloc(sizeof(image_t));
        image->filename = g_strdup(filename);
        image->img = (GtkImage*)gtk_image_new();
        /*gtk_image_set_from_pixbuf(image->img, pixbuf);*/
        gtk_image_set_from_file(image->img, filename);
        g_object_unref(pixbuf);
        g_array_append_val(images, image);
    }
    else
        warn("can't load %s", filename);
}

inline static void read_data(void)
{
    gchar** p = conf.infiles;
    for (register gint i = 0; p[i] && p ; i++)
    {
        read_image(p[i]);
    }
    for (register guint i = 0; i < images->len; i++)
    {
        image_t* img = g_array_index(images, image_t*, i);
        warn(img->filename);
    }
}

static gint luaI_quit(lua_State *L)
{
    exit(0);
}

static int luaI_next_image(lua_State *L)
{
    return 0;
}

static int luaI_prev_image(lua_State *L)
{
    return 0;
}

static int luaI_rotate(lua_State *L)
{
    return 0;
}

static gboolean lua_init()
{
    L = luaL_newstate();
    if (L == NULL)
        return FALSE;
    luaL_openlibs(L);
    lua_pushcfunction(L, luaI_next_image);
    lua_setglobal(L, "next_image");
    lua_pushcfunction(L, luaI_prev_image);
    lua_setglobal(L, "prev_image");
    lua_pushcfunction(L, luaI_quit);
    lua_setglobal(L, "quit");
    lua_pushcfunction(L, luaI_rotate);
    lua_setglobal(L, "rotate");
    return TRUE;
}

static gboolean lua_loadrc()
{
    const char* confpath = "./rc.lua";
    if (luaL_dofile(L, confpath) != 0)
    {
        fprintf(stderr, "couldn't parse config file \"%s\"\n", confpath);
        return FALSE;
    }
    return TRUE;
}

static void cb_key(GtkWidget *widget, GdkEventKey *ev, gpointer user_data)
{
    guint transformed_keyval = ev->keyval;
    gdk_keymap_translate_keyboard_state(
            gdk_keymap_get_default(),
            ev->hardware_keycode,
            ev->state,
            0, /* default group */
            &transformed_keyval, NULL, NULL, NULL);
    gchar ucs[7];
    guint ulen;
    guint32 ukval = gdk_keyval_to_unicode(transformed_keyval);
    if (g_unichar_isgraph(ukval))
    {
        ulen = g_unichar_to_utf8(ukval, ucs);
        ucs[ulen] = 0;
    }
    else
    {
        gchar *p = gdk_keyval_name(transformed_keyval);
        for (gint i = 0; *p; p++, i++)
            ucs[i] = *p;
    }

    int top = lua_gettop(L);
    lua_getglobal(L, "keys");
    lua_pushstring(L, ucs);

    if (lua_istable(L, -2))
    {
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1))
        {
            lua_pushstring(L, ucs);
            lua_pushnumber(L, transformed_keyval);
            lua_pushnumber(L, ev->keyval);
            lua_pcall(L, 3, 0, 0);
        }
    }
    lua_settop(L, top);
}

static void create_window()
{
    GtkWidget* box;

    window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), APPNAME);
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    /*image = (GtkImage*)gtk_image_new();*/
    /*open_image(selectedPicture);*/

    /*gtk_box_pack_start(GTK_BOX(box), (GtkWidget*)image, TRUE, TRUE, 0);*/
    /*gtk_container_add(GTK_CONTAINER(window), box);*/

    /*g_signal_connect(window, "key-press-event", G_CALLBACK(cb_key), NULL);*/
    g_signal_connect(window, "destroy",         G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all((GtkWidget*)window);
}

gint main(gint argc, gchar **argv)
{
    setlocale(LC_ALL, "");
    if (!lua_init())
        fatal("can't init Lua");
    if (!lua_loadrc())
        fatal("can't load rc.lua");
    parseopts(argc, argv);

    images = g_array_new(TRUE, FALSE, sizeof(image_t*));

    read_data();

    gtk_init(&argc, &argv);
    create_window();
    gtk_main();

    return 0;
}
