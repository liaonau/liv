#include "luaL.h"
#include "util.h"

#include <stdlib.h>

static int quit_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    if (gtk_main_level() != 0)
        gtk_main_quit();
    else
        exit(0);
    return 0;
}
static int resize_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    gint w = luaL_checknumber(L, 2);
    gint h = luaL_checknumber(L, 3);
    if (w > 0 && h > 0)
        gtk_window_resize(window, w, h);
    return 0;
}
static int content_size_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    GtkAllocation alloc;
    gtk_widget_get_allocation((GtkWidget*)content, &alloc);
    lua_pushnumber(L, alloc.width);
    lua_pushnumber(L, alloc.height);
    return 2;
}
static int window_size_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    gint w, h;
    gtk_window_get_size(window, &w, &h);
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
}
static int style_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    size_t size = 0;
    const gchar* data = luaL_checklstring(L, 2, &size);
    if (size == 0)
        return 0;
    guint priority = lua_toboolean(L, 3) ? GTK_STYLE_PROVIDER_PRIORITY_FALLBACK : GTK_STYLE_PROVIDER_PRIORITY_USER;
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, data, size, NULL);
    GdkDisplay* display = gdk_display_get_default();
    GdkScreen*  screen  = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), priority);
    g_object_unref(provider);
    return 0;
}
static int display_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    gpointer data = lua_touserdata(L, 2);
    GtkWidget* to_display;
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(content));
    if (data == NULL)
        to_display = NULL;
    else
    {
        lua_getmetatable(L, 2);
        luaL_getmetatable(L, UDATA_SCROLLL);
        luaL_getmetatable(L, UDATA_GRIDL);
        if (lua_equal(L, -3, -1))
            to_display = (GtkWidget*)(((gridL*)(data))->scroll);
        else if (lua_equal(L, -3, -2))
            to_display = (GtkWidget*)(((scrollL*)(data))->scroll);
        else
            to_display = NULL;
    }
    if (child && child != to_display)
        gtk_container_remove(GTK_CONTAINER(content), child);
    if (to_display)
        gtk_container_add(GTK_CONTAINER(content), to_display);
    gtk_widget_show_all((GtkWidget*)content);
    return 0;
}
static int mode_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(content));
    if (GTK_IS_GRID(child))
        lua_pushstring(L, LIB_GRIDL);
    else if (GTK_IS_SCROLLED_WINDOW(child))
        lua_pushstring(L, LIB_SCROLLL);
    else
        lua_pushnil(L);
    return 1;
}

static int is_file_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    const gchar* path = luaL_checkstring(L, 2);
    lua_pushboolean(L, g_file_test(path, G_FILE_TEST_IS_REGULAR));
    return 1;
}
static int xdg_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    lua_newtable(L);
    lua_pushstring(L, g_get_home_dir());
    lua_setfield(L, -2, "home");
    lua_pushstring(L, g_get_user_config_dir());
    lua_setfield(L, -2, "config");
    lua_pushstring(L, g_get_user_cache_dir());
    lua_setfield(L, -2, "cache");
    lua_pushstring(L, g_get_user_data_dir());
    lua_setfield(L, -2, "data");
    return 1;
}

static int index_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_STR( L, field, name,           APPNAME);
    CASE_STR( L, field, title,          gtk_window_get_title(window));
    CASE_BOOL(L, field, status_visible, gtk_widget_get_visible((GtkWidget*)statusbox));
    CASE_STR( L, field, status_left,    gtk_label_get_text(status_left));
    CASE_STR( L, field, status_right,   gtk_label_get_text(status_right));

    CASE_FUNC(L, field, window_size,  app);
    CASE_FUNC(L, field, content_size, app);
    CASE_FUNC(L, field, resize,       app);
    CASE_FUNC(L, field, quit,         app);
    CASE_FUNC(L, field, style,        app);
    CASE_FUNC(L, field, display,      app);
    CASE_FUNC(L, field, mode,         app);
    CASE_FUNC(L, field, is_file,      app);
    CASE_FUNC(L, field, xdg,          app);

    return 1;
}
static int newindex_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    const gchar* field = luaL_checkstring(L, 2);
    if (     g_strcmp0(field, "title") == 0)
    {
        const gchar* title = luaL_checkstring(L, 3);
        gtk_window_set_title(window, title);
    }
    else if (g_strcmp0(field, "status_visible") == 0)
    {
        gboolean visible = lua_toboolean(L, 3);
        gtk_widget_set_visible((GtkWidget*)statusbox, visible);
    }
    else if (g_strcmp0(field, "status_left") == 0)
    {
        const gchar* markup = luaL_checkstring(L, 3);
        gtk_label_set_markup(status_left, markup);
    }
    else if (g_strcmp0(field, "status_right") == 0)
    {
        const gchar* markup = luaL_checkstring(L, 3);
        gtk_label_set_markup(status_right, markup);
    }
    return 0;
}
static int tostring_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    lua_pushstring(L, APPNAME);
    return 1;
}
static const struct luaL_Reg appLlib_m [] =
{
    {"__tostring", tostring_appL},
    {"__index",    index_appL   },
    {"__newindex", newindex_appL},
    {NULL,         NULL         }
};
int luaopen_appL(lua_State *L, const gchar* name)
{
    lua_newuserdata(L, sizeof(appL));
    luaL_newmetatable(L, UDATA_APPL);
    luaL_setfuncs(L, appLlib_m, 0);
    lua_setmetatable(L, -2);
    lua_setglobal(L, name);
    return 1;
}
