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

#include "luaL.h"
#include "gridL.h"
#include "frameL.h"
#include "util.h"

static int new_gridL(lua_State* L)
{
    gridL* g = (gridL*)lua_newuserdata(L, sizeof(gridL));

    g->grid   = (GtkGrid*)gtk_grid_new();
    g_object_ref(g->grid);

    gtk_grid_set_row_spacing(   g->grid, 0);
    gtk_grid_set_column_spacing(g->grid, 0);
    gtk_widget_set_valign((GtkWidget*)g->grid, GTK_ALIGN_CENTER);
    gtk_widget_set_halign((GtkWidget*)g->grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_homogeneous(   g->grid, FALSE);
    gtk_grid_set_column_homogeneous(g->grid, FALSE);

    luaL_getmetatable(L, GRID);
    lua_setmetatable(L, -2);
    return 1;
}

static void clear_cb(GtkWidget* w, gpointer data)
{
    gtk_container_remove(GTK_CONTAINER(data), w);
}
static int clear_gridL(lua_State* L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    gtk_container_foreach(GTK_CONTAINER(g->grid), clear_cb, (gpointer)g->grid);
    gtk_widget_show_all((GtkWidget*)g->grid);
    return 0;
}
static int detach_gridL(lua_State* L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    gint l = luaL_checkinteger(L, 2);
    gint t = luaL_checkinteger(L, 3);
    GtkWidget* c = gtk_grid_get_child_at(g->grid, l, t);
    if (GTK_IS_FRAME(c))
        gtk_container_remove(GTK_CONTAINER(g->grid), c);
    gtk_widget_show_all((GtkWidget*)g->grid);
    return 0;
}
static int attach_gridL(lua_State* L)
{
    gridL*  g = (gridL*)luaL_checkudata(L, 1, GRID);
    frameL* f = (frameL*)luaL_checkudata(L, 2, FRAME);
    gint l = luaL_checkinteger(L, 3);
    gint t = luaL_checkinteger(L, 4);
    GtkWidget* c = gtk_grid_get_child_at(g->grid, l, t);
    if (f->frame != (GtkFrame*)c)
    {
        if (GTK_IS_FRAME(c))
            gtk_container_remove(GTK_CONTAINER(g->grid), c);
        gtk_grid_attach(g->grid, (GtkWidget*)f->frame, l, t, 1, 1);
    }
    gtk_widget_show_all((GtkWidget*)g->grid);
    return 0;
}

static int gc_gridL(lua_State* L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, GRID);
    g_object_unref(g->grid);
    return 0;
}
static int tostring_gridL(lua_State* L)
{
    luaL_checkudata(L, 1, GRID);
    lua_pushstring(L, GRID);
    return 1;
}
static int index_gridL(lua_State* L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, GRID);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_FUNC( detach,   grid );
    CASE_FUNC( clear,    grid );
    CASE_FUNC( attach,   grid );

    if (g_strcmp0(field, "spacing") == 0)
    {
        lua_newtable(L);
        lua_pushnumber(L, gtk_grid_get_row_spacing(g->grid));
        lua_setfield(L, -2, "row");
        lua_pushnumber(L, gtk_grid_get_column_spacing(g->grid));
        lua_setfield(L, -2, "column");
    }
    return 1;
}
static int newindex_gridL(lua_State* L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, GRID);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "spacing") == 0)
    {
        if (lua_istable(L, 3))
        {
            lua_getfield(L, 3, "row");
            if (lua_isnumber(L, -1))
            {
                gint r = lua_tonumber(L, -1);
                if (r >= 0)
                    gtk_grid_set_row_spacing(g->grid,   r);
            }
            lua_getfield(L, 3, "column");
            if (lua_isnumber(L, -1))
            {
                gint c = lua_tonumber(L, -1);
                if (c >= 0)
                    gtk_grid_set_column_spacing(g->grid,   c);
            }
        }
    }
    return 0;
}

static const struct luaL_Reg gridLlib_f [] =
{
    {"new", new_gridL},
    {NULL,  NULL     }
};
static const struct luaL_Reg gridLlib_m [] =
{
    {"__gc",       gc_gridL      },
    {"__tostring", tostring_gridL},
    {"__index",    index_gridL   },
    {"__newindex", newindex_gridL},
    {NULL,         NULL          }
};
int luaopen_gridL(lua_State *L)
{
    luaL_newmetatable(L, GRID);
    luaL_setfuncs(L, gridLlib_m, 0);
    luaL_newlib(L, gridLlib_f, GRID);
    return 1;
}
