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

#include <stdlib.h>

#include "luaL.h"
#include "util.h"
#include "appL.h"
#include "gridL.h"
#include "frameL.h"

static int quit_appL(lua_State *L)
{
    if (gtk_main_level() != 0)
        gtk_main_quit();
    else
        exit(0);
    return 0;
}
static int style_appL(lua_State *L)
{
    size_t size = 0;
    const gchar* data = luaL_checklstring(L, 1, &size);
    if (size == 0)
        return 0;
    guint priority = lua_toboolean(L, 2) ? GTK_STYLE_PROVIDER_PRIORITY_FALLBACK : GTK_STYLE_PROVIDER_PRIORITY_USER;
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, data, size, NULL);
    GdkDisplay* display = gdk_display_get_default();
    GdkScreen*  screen  = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), priority);
    g_object_unref(provider);
    return 0;
}

static int resize_appL(lua_State *L)
{
    gint w = luaL_checknumber(L, 1);
    gint h = luaL_checknumber(L, 2);
    if (w > 0 && h > 0)
        gtk_window_resize(window, w, h);
    return 0;
}
static int window_size_appL(lua_State *L)
{
    gint w, h;
    gtk_window_get_size(window, &w, &h);
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
}
static int content_size_appL(lua_State *L)
{
    GtkAllocation alloc;
    gtk_widget_get_allocation((GtkWidget*)scroll, &alloc);
    lua_pushnumber(L, alloc.width);
    lua_pushnumber(L, alloc.height);
    return 2;
}

static int title_get_appL(lua_State *L)
{
    lua_pushstring(L, gtk_window_get_title(window));
    return 1;
}
static int title_set_appL(lua_State *L)
{
    const gchar* title = luaL_checkstring(L, 2);
    gtk_window_set_title(window, title);
    return 0;
}

static int status_get_appL(lua_State *L)
{
    lua_newtable(L);
    lua_pushboolean(L, gtk_widget_get_visible((GtkWidget*)statusbox));
    lua_setfield(L, -2, "visible");
    lua_pushstring(L, gtk_label_get_text(status_left));
    lua_setfield(L, -2, "left");
    lua_pushstring(L, gtk_label_get_text(status_right));
    lua_setfield(L, -2, "right");
    return 1;
}
static int status_set_appL(lua_State *L)
{
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "visible");
        gtk_widget_set_visible((GtkWidget*)statusbox, lua_toboolean(L, -1));
        lua_pop(L, 1);
        lua_getfield(L, 2, "left");
        gtk_label_set_markup(status_left,  luaL_optstring(L, -1, ""));
        lua_pop(L, 1);
        lua_getfield(L, 2, "right");
        gtk_label_set_markup(status_right, luaL_optstring(L, -1, ""));
        lua_pop(L, 1);
    }
    else if (lua_isboolean(L, 2))
        gtk_widget_set_visible((GtkWidget*)statusbox, lua_toboolean(L, 2));
    return 0;
}

static int scroll_get_appL(lua_State *L)
{
    GtkAdjustment* hor_adjustment = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)scroll);
    GtkAdjustment* ver_adjustment = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)scroll);
    lua_newtable(L);
#define GET_ADJ(direction, type, field)                            \
    {                                                              \
        gdouble val;                                               \
        val = gtk_adjustment_get_##type(direction##_##adjustment); \
        lua_pushnumber(L, val);                                    \
        lua_setfield(L, -2, field);                                \
    }
    GET_ADJ(hor, value, "h");
    GET_ADJ(ver, value, "v");
    GET_ADJ(hor, lower, "min_h");
    GET_ADJ(ver, lower, "min_v");
    GET_ADJ(hor, upper, "max_h");
    GET_ADJ(ver, upper, "max_v");
    return 1;
}
static int scroll_set_appL(lua_State *L)
{
    if (lua_istable(L, 2))
    {
        GtkAdjustment* hor_adjustment = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)scroll);
        GtkAdjustment* ver_adjustment = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)scroll);
#define SET_ADJ(direction, type, field)                                   \
        {                                                                 \
            gdouble tmp, val;                                             \
            tmp = gtk_adjustment_get_##type(direction##_##adjustment);    \
            lua_getfield(L, 2, field);                                    \
            val = luaL_optnumber(L, -1, tmp);                             \
            lua_pop(L, 1);                                                \
            if (tmp != val)                                               \
                gtk_adjustment_set_##type(direction##_##adjustment, val); \
        }

        SET_ADJ(hor, value, "h");
        SET_ADJ(ver, value, "v");
        SET_ADJ(hor, lower, "min_h");
        SET_ADJ(ver, lower, "min_v");
        SET_ADJ(hor, upper, "max_h");
        SET_ADJ(ver, upper, "max_v");
    }
    return 0;
}

static int display_get_appL(lua_State *L)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, displayref);
    return 1;
}
static int display_set_appL(lua_State *L)
{
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(scroll));
    if (child)
    {
        gtk_container_remove(GTK_CONTAINER(scroll), child);
        luaL_unref(L, LUA_REGISTRYINDEX, displayref);
    }

    GtkWidget* to_display = NULL;

    gpointer data = lua_touserdata(L, 2);
    lua_getmetatable(L, 2);
    luaL_getmetatable(L, GRID);
    luaL_getmetatable(L, FRAME);
    if (lua_equal(L, -3, -2))
        to_display = (GtkWidget*)(((gridL*)(data))->grid);
    else if (lua_equal(L, -3, -1))
        to_display = (GtkWidget*)(((frameL*)(data))->frame);
    if (to_display)
    {
        lua_pushvalue(L, 2);
        displayref = luaL_ref(L, LUA_REGISTRYINDEX);
        gtk_container_add(GTK_CONTAINER(scroll), to_display);
    }
    gtk_widget_show_all((GtkWidget*)scroll);
    return 0;
}

static int is_file_appL(lua_State *L)
{
    const gchar* path = luaL_checkstring(L, 1);
    lua_pushboolean(L, g_file_test(path, G_FILE_TEST_IS_REGULAR));
    return 1;
}
static int is_dir_appL(lua_State *L)
{
    const gchar* path = luaL_checkstring(L, 1);
    lua_pushboolean(L, g_file_test(path, G_FILE_TEST_IS_DIR));
    return 1;
}
static int mk_dir_appL(lua_State *L)
{
    const gchar* path = luaL_checkstring(L, 1);
    gint         mode = luaL_checknumber(L, 2);

    gint ret = g_mkdir_with_parents(path, mode);
    lua_pushboolean(L, (ret == 0));
    return 1;
}
static int xdg_appL(lua_State *L)
{
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
static int digest_appL(lua_State *L)
{
    const gchar* path = luaL_checkstring(L, 1);
    gchar* contents = NULL;
    gsize  length;
    gboolean success = g_file_get_contents(path, &contents, &length, NULL);
    if (success)
    {
        GChecksum* cs = g_checksum_new(G_CHECKSUM_MD5);
        g_checksum_update(cs, (guchar*)contents, length);
        const gchar* md5 = g_checksum_get_string(cs);
        lua_pushstring(L, md5);
        g_checksum_free(cs);
    }
    else
        lua_pushnil(L);
    g_free(contents);
    return 1;
}
static int hash_appL(lua_State *L)
{
    const gchar* path = luaL_checkstring(L, 1);
    GFile* file = g_file_new_for_path(path);
    guint hash = g_file_hash(file);
    lua_pushnumber(L, hash);
    g_object_unref(file);
    return 1;
}

static int index_appL(lua_State *L)
{
    const gchar* field = luaL_checkstring(L, 2);

    CASE_STR(    name,    APPNAME);

    INDEX_FIELD( title,   app );
    INDEX_FIELD( status,  app );
    INDEX_FIELD( scroll,  app );
    INDEX_FIELD( display, app );

    CASE_FUNC( quit,    app );
    CASE_FUNC( style,   app );
    CASE_FUNC( is_file, app );
    CASE_FUNC( is_dir,  app );
    CASE_FUNC( mk_dir,  app );
    CASE_FUNC( xdg,     app );
    CASE_FUNC( digest,  app );
    CASE_FUNC( hash,    app );

    CASE_FUNC( resize,       app );
    CASE_FUNC( window_size,  app );
    CASE_FUNC( content_size, app );

    return 1;
}
static int newindex_appL(lua_State *L)
{
    const gchar* field = luaL_checkstring(L, 2);
    NEWINDEX_FIELD( title,   app );
    NEWINDEX_FIELD( status,  app );
    NEWINDEX_FIELD( scroll,  app );
    NEWINDEX_FIELD( display, app );
    return 0;
}
static int tostring_appL(lua_State *L)
{
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
int luaopen_appL(lua_State *L)
{
    lua_newtable(L);
    luaL_newmetatable(L, APP);
    luaL_setfuncs(L, appLlib_m, 0);
    lua_setmetatable(L, -2);
    lua_setglobal(L, APP);
    return 1;
}
