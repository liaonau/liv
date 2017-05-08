#include "conf.h"
#include "util.h"
#include "luah.h"

#include <math.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <glib/gprintf.h>

GtkWindow* window;
GtkWidget* swindow;
GtkWidget* grid;
GtkImage*  current;
gboolean   preview;

void read_image(const gchar* filename)
{
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if (GDK_IS_PIXBUF(pixbuf))
    {
        image_t* image  = g_malloc(sizeof(image_t));
        image->filename = g_strdup(filename);
        image->thumb    = (GtkImage*)gtk_image_new_from_pixbuf(pixbuf);
        image->size.x   = gdk_pixbuf_get_width(pixbuf);
        image->size.y   = gdk_pixbuf_get_height(pixbuf);
        image->index    = files->len;
        image->marked   = FALSE;
        g_object_unref(pixbuf);
        g_array_append_val(files, image);
    }
    else
        warn("can't load %s", filename);
}

static gint calculate_max_thumb_size(void)
{
    gint mats = 0;
    for (guint i = 0; i < files->len; i++)
    {
        image_t* image = g_array_index(files, image_t*, i);
        point s = image->size;
        mats = MAX(mats, s.x);
        mats = MAX(mats, s.y);
    }
    return MIN(mats, conf.max_thumb_size);
}

static point transform_with_aspect(point p, point m)
{
    double aspect = p.x/(double)(p.y);
    if ((p.x > m.x) || (p.y > m.y))
    {
        point n = {m.x, m.y};
        if (p.x > m.x)
        {
            n.x = m.x;
            n.y = (gint)(n.x/aspect);
            if (n.y > m.y)
            {
                n.y = m.y;
                n.x = (gint)(n.y*aspect);
            }
        }
        if (p.y > m.y)
        {
            n.y = m.y;
            n.x = (gint)(n.y*aspect);
            if (n.x > m.x)
            {
                n.x = m.x;
                n.y = (gint)(n.x/aspect);
            }
        }
        return n;
    }
    return p;
}

static void set_previews_size(void)
{
    gint mts = calculate_max_thumb_size();
    for (guint i = 0; i < files->len; i++)
    {
        image_t* image = g_array_index(files, image_t*, i);
        GtkImage* img = image->thumb;
        point    size = image->size;
        if (size.x > mts || size.y > mts)
        {
            point max_p = {mts, mts};
            point new_p = transform_with_aspect(size, max_p);
            GdkPixbuf* old_pixbuf = gtk_image_get_pixbuf(img);
            if (GDK_IS_PIXBUF(old_pixbuf))
            {
                GdkPixbuf* new_pixbuf = gdk_pixbuf_scale_simple(old_pixbuf, new_p.x, new_p.y, GDK_INTERP_BILINEAR);
                if (GDK_IS_PIXBUF(new_pixbuf))
                    gtk_image_set_from_pixbuf(img, new_pixbuf);
            }
        }
    }
}

static void fill_grid(void)
{
    gint square = ceil(sqrt(files->len));
    gint width  = 0;
    gint height = 0;
    for (guint i = 0; i < files->len; i++)
    {
        image_t* image = g_array_index(files, image_t*, i);
        GtkImage* img = image->thumb;
        GdkPixbuf* pixbuf = gtk_image_get_pixbuf(img);
        gint pw, ph;
        g_object_get(pixbuf, "width", &pw, "height", &ph, NULL);
        if (width < pw)
            width = pw;
        if (height < ph)
            height = ph;
    }
    for (guint i = 0; i < files->len; i++)
    {
        image_t* image = g_array_index(files, image_t*, i);
        GtkImage* img = image->thumb;
        gint left = i % square;
        gint top  = (i - (i % square)) / square;
        gtk_grid_attach((GtkGrid*)grid, (GtkWidget*)img, left, top, 1, 1);
    }
    set_previews_size();
}

static void read_files(void)
{
    gchar** p = infiles;
    for (gint i = 0; p[i] && p ; i++)
        read_image(p[i]);
}

static gint luaI_quit(lua_State *L)
{
    exit(0);
}

static void clear_swindow(GtkWidget* widget, gpointer cbdta)
{
    gtk_container_remove(GTK_CONTAINER(swindow), widget);
}

static void load_image()
{
    image_t* image = g_array_index(files, image_t*, pointer);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(image->filename, NULL);
    if (GDK_IS_PIXBUF(pixbuf))
    {
        current = (GtkImage*)gtk_image_new_from_pixbuf(pixbuf);
        g_object_unref(pixbuf);
    }
    else
        current = (GtkImage*)gtk_image_new();
    gtk_container_foreach(GTK_CONTAINER(swindow), clear_swindow, NULL);
    gtk_container_add(GTK_CONTAINER(swindow), (GtkWidget*)current);
    gtk_widget_show((GtkWidget*)current);
}

static int luaI_set_image(lua_State *L)
{
    gint prev_pointer = pointer;
    gint len = files->len;
    gboolean relative = lua_toboolean(L, 1);
    if (relative)
    {
        gint step = lua_tointeger(L, 2);
        pointer += step;
        if (pointer >= len)
            pointer = len - 1;
        else if (pointer < 0)
            pointer = 0;
    }
    else
    {
        gint p = lua_tointeger(L, 2);
        if (p > len)
            p = len - 1;
        else if (p < 0 && p >= -len)
            p = len + p;
        else
            p = 0;
        pointer = p;
    }
    if (!preview)
    {
        if (prev_pointer != pointer)
            load_image();
    }
    return 0;
}

static int luaI_flip(lua_State *L)
{
    if (preview)
        return 0;
    gboolean horizontal = lua_toboolean(L, 1);
    GdkPixbuf* old_pixbuf = gtk_image_get_pixbuf(current);
    if (GDK_IS_PIXBUF(old_pixbuf))
    {
        GdkPixbuf* new_pixbuf = gdk_pixbuf_flip(old_pixbuf, horizontal);
        if (GDK_IS_PIXBUF(new_pixbuf))
            gtk_image_set_from_pixbuf(current, new_pixbuf);
    }
    return 0;
}

static int luaI_scale(lua_State *L)
{
    if (preview)
        return 0;
    GdkPixbuf* old_pixbuf = gtk_image_get_pixbuf(current);
    if (GDK_IS_PIXBUF(old_pixbuf))
    {
        point ws = conf.window_size;
        point is = {gdk_pixbuf_get_width(old_pixbuf), gdk_pixbuf_get_height(old_pixbuf)};
        point ns = transform_with_aspect(is, ws);
        if (is.x != ns.x || is.y != ns.y)
        {
            GdkPixbuf* new_pixbuf = gdk_pixbuf_scale_simple(old_pixbuf, ns.x, ns.y, GDK_INTERP_BILINEAR);
            if (GDK_IS_PIXBUF(new_pixbuf))
                gtk_image_set_from_pixbuf(current, new_pixbuf);
        }
    }
    return 0;
}

static int luaI_rotate(lua_State *L)
{
    if (preview)
        return 0;
    GdkPixbufRotation rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
    gboolean clockwise = lua_toboolean(L, 1);
    if (!clockwise)
        rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(current);
    if (GDK_IS_PIXBUF(pixbuf))
        gtk_image_set_from_pixbuf(current, gdk_pixbuf_rotate_simple(pixbuf, rotation));
    return 0;
}

static int luaI_slide(lua_State *L)
{
    gdouble vertical = lua_toboolean(L, 1);
    gdouble step = lua_tonumber(L, 2);
    GtkAdjustment* (*fn)(GtkScrolledWindow*) = &gtk_scrolled_window_get_hadjustment;
    if (vertical)
        fn = &gtk_scrolled_window_get_vadjustment;

    GtkAdjustment* adj = fn((GtkScrolledWindow*)swindow);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) + step);
    return 0;
}

static int luaI_get_preivew(lua_State *L)
{
    lua_pushboolean(L, preview);
    return 1;
}

static int luaI_toggle_preview(lua_State *L)
{
    preview = !preview;
    gtk_container_foreach(GTK_CONTAINER(swindow), clear_swindow, NULL);
    if (preview)
    {
        gtk_container_add(GTK_CONTAINER(swindow), grid);
    }
    else
    {
        load_image();
    }
    return 0;
}

void luaH_init_functions(void)
{
    lua_pushcfunction(L, luaI_set_image);
    lua_setglobal(L, "set_image");
    lua_pushcfunction(L, luaI_quit);
    lua_setglobal(L, "quit");
    lua_pushcfunction(L, luaI_rotate);
    lua_setglobal(L, "rotate");
    lua_pushcfunction(L, luaI_slide);
    lua_setglobal(L, "slide");
    lua_pushcfunction(L, luaI_toggle_preview);
    lua_setglobal(L, "toggle_preview");
    lua_pushcfunction(L, luaI_get_preivew);
    lua_setglobal(L, "is_preview");
    lua_pushcfunction(L, luaI_scale);
    lua_setglobal(L, "scale");
    lua_pushcfunction(L, luaI_flip);
    lua_setglobal(L, "flip");
}

void luaH_read_conf(void)
{
    conf.max_thumb_size = 128;
    conf.window_size.x  = 800;
    conf.window_size.y  = 600;
    lua_getglobal(L, "conf");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "max_thumb_size");
        conf.max_thumb_size = luaL_optinteger(L, -1, conf.max_thumb_size);
        lua_pop(L, 1);
        lua_getfield(L, -1, "window_size");
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "width");
            conf.window_size.x = luaL_optinteger(L, -1, conf.window_size.x);
            lua_pop(L, 1);
            lua_getfield(L, -1, "height");
            conf.window_size.y = luaL_optinteger(L, -1, conf.window_size.y);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

static gboolean luaH_call_keypress_cb(keypress_t* keypress)
{
    lua_getfield(L, -1, keypress->keyname);
    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, keypress->keyname);
        lua_pushnumber(L, keypress->transformed_keyval);
        lua_pushnumber(L, keypress->keyval);
        lua_pcall(L, 3, 0, 0);
        return TRUE;
    }
    return FALSE;
}

static void cb_key(GtkWidget *widget, GdkEventKey *ev, gpointer user_data)
{
    keypress_t* keypress = keyval_to_keypress(ev);

    int top = lua_gettop(L);
    lua_getglobal(L, "keys");
    if (lua_istable(L, -1))
    {
        if (preview)
            lua_getfield(L, -1, "preview");
        else
            lua_getfield(L, -1, "image");
        if (lua_istable(L, -1))
        {
            if (!luaH_call_keypress_cb(keypress))
            {
                lua_pop(L, 2);
                luaH_call_keypress_cb(keypress);
            }
        }
    }
    g_free(keypress);
    lua_settop(L, top);
}

static void cb_size(GtkWidget *widget, GdkRectangle *rect, gpointer user_data)
{
    gboolean changed = FALSE;
    if (conf.window_size.x != rect->width)
    {
        conf.window_size.x = rect->width;
        changed = TRUE;
    }
    if (conf.window_size.y != rect->height)
    {
        conf.window_size.y = rect->height;
        changed = TRUE;
    }
    if (changed)
    {
        /*warn("%d %d %d %d", rect->x, rect->y, rect->width, rect->height);*/
        set_previews_size();
    }
}

gint main(gint argc, gchar **argv)
{
    setlocale(LC_ALL, "");
    if (argc < 2)
        fatal("nothing to display");
    execpath = g_strdup(argv[0]);
    infiles  = argv + 1;

    if (!luaH_init())
        fatal("can't init Lua");
    luaH_init_functions();
    if (!luaH_loadrc())
        fatal("can't load rc.lua");
    luaH_read_conf();

    gtk_init(&argc, &argv);

    window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), APPNAME);
    gtk_window_set_default_size(GTK_WINDOW(window), conf.window_size.x, conf.window_size.y);

    swindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), swindow);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing((GtkGrid*)grid, 10);
    gtk_grid_set_row_spacing((GtkGrid*)grid, 10);
    gtk_grid_set_column_homogeneous((GtkGrid*)grid, TRUE);
    gtk_grid_set_row_homogeneous((GtkGrid*)grid, TRUE);
    g_object_ref(grid);

    pointer = 0;
    files = g_array_new(TRUE, FALSE, sizeof(image_t*));

    read_files();
    fill_grid();

    preview = TRUE;
    gtk_container_add(GTK_CONTAINER(swindow), grid);

    g_signal_connect(window, "size-allocate",   G_CALLBACK(cb_size),       NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(cb_key),        NULL);
    g_signal_connect(window, "destroy",         G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all((GtkWidget*)window);
    gtk_main();

    return 0;
}
