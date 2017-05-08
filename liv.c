#include <stdlib.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "filenamenode.h"

const char* programname = "lua image viewer";
const char* shortprogramname = "liv";

struct FilenameNode* selectedPicture;
GtkWindow* window;
GtkImage* image;
lua_State *L;

static gboolean open_image(struct FilenameNode *filenamenode)
{
    gtk_image_set_from_file(image, filenamenode->filename);

    const GdkPixbuf *pb = gtk_image_get_pixbuf(image);
    if (pb != NULL)
    {
        /*gtk_window_resize(window, gdk_pixbuf_get_width(pb), gdk_pixbuf_get_height(pb));*/
        gtk_window_set_title(GTK_WINDOW(window), filenamenode->filename);
        return TRUE;
    }
    else
    {
        g_printf("Couldn't open %s\n", filenamenode->filename);
        deleteFilenameNode(filenamenode);
        return FALSE;
    }
}

void clear()
{
    deleteAllFilenameNodes(selectedPicture);
}

static gint luaI_quit(lua_State *L)
{
    clear();
    exit(0);
}

static int luaI_next_image(lua_State *L)
{
    selectedPicture = selectedPicture->next;
    while(!open_image(selectedPicture))
        selectedPicture = selectedPicture->next;
    return 0;
}

static int luaI_prev_image(lua_State *L)
{
    selectedPicture = selectedPicture->prev;
    while(!open_image(selectedPicture))
        selectedPicture = selectedPicture->prev;
    return 0;
}

static int luaI_rotate(lua_State *L)
{
    GdkPixbufRotation rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
    gboolean clockwise = lua_toboolean(L, 1);
    if (!clockwise)
        rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
    gtk_image_set_from_pixbuf(image, gdk_pixbuf_rotate_simple(gtk_image_get_pixbuf(image), rotation));
    return 0;
}

static gboolean lua_init()
{
    L = luaL_newstate();
    if (L == NULL)
    {
        g_fprintf(stderr, "%s\n", "couldn't initialize Lua");
        return FALSE;
    }
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
    gtk_window_set_title(GTK_WINDOW(window), programname);
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    image = (GtkImage*)gtk_image_new();
    open_image(selectedPicture);

    gtk_box_pack_start(GTK_BOX(box), (GtkWidget*)image, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    g_signal_connect(window, "key-press-event", G_CALLBACK(cb_key), NULL);
    g_signal_connect(window, "destroy",         G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all((GtkWidget*)window);
}

gint main(gint argc, gchar **argv)
{
    if (argc < 2)
    {
        g_printf("Please specify a image to display\n");
        return 1;
    }
    if (!lua_init())
        return 1;
    if (!lua_loadrc())
        return 1;

    loadFilenames(argc, argv);
    gtk_init(&argc, &argv);
    create_window();
    gtk_main();

    clear();
    return 0;
}
