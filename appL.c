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

static int index_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    const gchar* field = luaL_checkstring(L, 2);
    if (     g_strcmp0(field, "name") == 0)
        lua_pushstring(L, APPNAME);
    else if (g_strcmp0(field, "title") == 0)
        lua_pushstring(L, gtk_window_get_title(window));
    else if (g_strcmp0(field, "width") == 0)
    {
        gint w, h;
        gtk_window_get_size(window, &w, &h);
        lua_pushnumber(L, w);
    }
    else if (g_strcmp0(field, "height") == 0)
    {
        gint w, h;
        gtk_window_get_size(window, &w, &h);
        lua_pushnumber(L, h);
    }
    else if (g_strcmp0(field, "hscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)scroll);
        gdouble value = gtk_adjustment_get_value(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "vscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)scroll);
        gdouble value = gtk_adjustment_get_value(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "resize") == 0)
        lua_pushcfunction(L, resize_appL);
    else if (g_strcmp0(field, "quit") == 0)
        lua_pushcfunction(L, quit_appL);
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
    else if (g_strcmp0(field, "width") == 0)
    {
        gint w = luaL_checknumber(L, 3);
        if (w > 0)
        {
            gint w_aux, h;
            gtk_window_get_size(window, &w_aux, &h);
            gtk_window_resize(window, w, h);
        }
    }
    else if (g_strcmp0(field, "height") == 0)
    {
        gint h = luaL_checknumber(L, 3);
        if (h > 0)
        {
            gint w, h_aux;
            gtk_window_get_size(window, &w, &h_aux);
            gtk_window_resize(window, w, h);
        }
    }
    else if (g_strcmp0(field, "hscroll") == 0)
    {
        gdouble value = luaL_checknumber(L, 3);
        GtkAdjustment* adj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)scroll);
        gtk_adjustment_set_value(adj, value);
    }
    else if (g_strcmp0(field, "vscroll") == 0)
    {
        gdouble value = luaL_checknumber(L, 3);
        GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)scroll);
        gtk_adjustment_set_value(adj, value);
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
    /*gint ref = lua_ref(L, LUA_REGISTRYINDEX);*/
    /*lua_rawgeti(L, LUA_REGISTRYINDEX, ref);*/
    luaL_newmetatable(L, UDATA_APPL);
    luaL_setfuncs(L, appLlib_m, 0);
    lua_setmetatable(L, -2);
    lua_setglobal(L, name);
    return 1;
}

