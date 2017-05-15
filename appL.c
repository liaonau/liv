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


static int index_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    const gchar* field = luaL_checkstring(L, 2);
    if (     g_strcmp0(field, "name") == 0)
        lua_pushstring(L, APPNAME);
    else if (g_strcmp0(field, "title") == 0)
        lua_pushstring(L, gtk_window_get_title(window));
    else if (g_strcmp0(field, "window_size") == 0)
        lua_pushcfunction(L, window_size_appL);
    else if (g_strcmp0(field, "content_size") == 0)
        lua_pushcfunction(L, content_size_appL);
    else if (g_strcmp0(field, "status_visible") == 0)
    {
        gboolean visible = gtk_widget_get_visible((GtkWidget*)statusbox);
        lua_pushboolean(L, visible);
    }
    else if (g_strcmp0(field, "status_left") == 0)
        lua_pushstring(L, gtk_label_get_text(status_left));
    else if (g_strcmp0(field, "status_right") == 0)
        lua_pushstring(L, gtk_label_get_text(status_right));
    else if (g_strcmp0(field, "resize") == 0)
        lua_pushcfunction(L, resize_appL);
    else if (g_strcmp0(field, "quit") == 0)
        lua_pushcfunction(L, quit_appL);
    else if (g_strcmp0(field, "style") == 0)
        lua_pushcfunction(L, style_appL);
    else
        lua_pushnil(L);
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

