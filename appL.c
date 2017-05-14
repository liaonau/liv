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

static int style_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    size_t size = 0;
    const gchar* data = luaL_checklstring(L, 2, &size);
    if (size == 0)
        return 0;
    guint priority = lua_toboolean(L, 3) ? GTK_STYLE_PROVIDER_PRIORITY_FALLBACK : GTK_STYLE_PROVIDER_PRIORITY_USER;
    GtkCssProvider* provider = gtk_css_provider_new();
    if (!gtk_css_provider_load_from_data(provider, data, size, NULL))
        return 0;
    GdkDisplay* display = gdk_display_get_default();
    GdkScreen*  screen  = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), priority);
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
        GtkAllocation alloc;
        gtk_widget_get_allocation((GtkWidget*)scroll, &alloc);
        lua_pushnumber(L, alloc.width);
    }
    else if (g_strcmp0(field, "height") == 0)
    {
        GtkAllocation alloc;
        gtk_widget_get_allocation((GtkWidget*)scroll, &alloc);
        lua_pushnumber(L, alloc.height);
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
    else if (g_strcmp0(field, "max_hscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)scroll);
        gdouble value = gtk_adjustment_get_upper(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "max_vscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)scroll);
        gdouble value = gtk_adjustment_get_upper(adj);
        lua_pushnumber(L, value);
    }
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

