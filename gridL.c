#include "luaL.h"
#include "gridL.h"
#include "graph.h"
#include "imageL.h"
#include "util.h"

static int new_gridL(lua_State *L)
{
    gridL* g = (gridL*)lua_newuserdata(L, sizeof(gridL));
    g->grid  = (GtkGrid*)gtk_grid_new();
    g_object_ref(g->grid);
    GtkGrid* grid = g->grid;
    gtk_grid_set_column_spacing(grid, 10);
    gtk_grid_set_row_spacing(grid, 10);
    gtk_grid_set_column_homogeneous(grid, TRUE);
    gtk_grid_set_row_homogeneous(grid, TRUE);

    luaL_getmetatable(L, UDATA_GRIDL);
    lua_setmetatable(L, -2);
    return 1;
}

static int clear_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    GtkGrid* grid = g->grid;
    gtk_container_foreach(GTK_CONTAINER(grid), container_clear_cb, grid);
    gtk_widget_show_all((GtkWidget*)grid);
    return 0;
}

static int attach_gridL(lua_State *L)
{
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    imageL* i = (imageL*)luaL_checkudata(L, 2, UDATA_IMAGEL);
    gint left = luaL_optint(L, 3, 1);
    gint top  = luaL_optint(L, 4, 1);
    if (left <= 0 || top <= 0)
        return 0;
    GtkGrid* grid   = g->grid;
    GtkImage* image = i->image;
    gtk_grid_attach(grid, (GtkWidget*)image, left, top, 1, 1);
    gtk_widget_show_all((GtkWidget*)grid);
    return 0;
}

static int add_gridL(lua_State *L)
{
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    imageL* i = (imageL*)luaL_checkudata(L, 2, UDATA_IMAGEL);
    GtkGrid* grid   = g->grid;
    GtkImage* image = i->image;
    gtk_container_add(GTK_CONTAINER(grid), (GtkWidget*)image);
    gtk_widget_show_all((GtkWidget*)grid);
    return 0;
}

static int show_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    GtkGrid* grid = g->grid;
    gtk_container_foreach(GTK_CONTAINER(scroll), container_clear_cb, scroll);
    gtk_container_add(GTK_CONTAINER(scroll), (GtkWidget*)grid);
    gtk_widget_show_all((GtkWidget*)grid);
    return 0;
}

static int gc_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    g_object_unref(g->grid);
    return 0;
}

static int index_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    GtkGrid* grid = g->grid;
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "homogeneous") == 0)
    {
        lua_newtable(L);
        lua_pushboolean(L, gtk_grid_get_column_homogeneous(grid));
        lua_setfield(L, -2, "column");
        lua_pushboolean(L, gtk_grid_get_row_homogeneous(grid));
        lua_setfield(L, -2, "row");
    }
    else if (g_strcmp0(field, "spacing") == 0)
    {
        lua_newtable(L);
        lua_pushnumber(L, gtk_grid_get_column_spacing(grid));
        lua_setfield(L, -2, "column");
        lua_pushnumber(L, gtk_grid_get_row_spacing(grid));
        lua_setfield(L, -2, "row");
    }
    else if (g_strcmp0(field, "clear") == 0)
        lua_pushcfunction(L, clear_gridL);
    else if (g_strcmp0(field, "attach") == 0)
        lua_pushcfunction(L, attach_gridL);
    else if (g_strcmp0(field, "add") == 0)
        lua_pushcfunction(L, add_gridL);
    else if (g_strcmp0(field, "show") == 0)
        lua_pushcfunction(L, show_gridL);
    else
        lua_pushnil(L);
    return 1;
}

static int newindex_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    GtkGrid* grid = g->grid;
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "homogeneous") == 0)
    {
        if (lua_istable(L, 3))
        {
            lua_getfield(L, 3, "column");
            lua_getfield(L, 3, "row");
            gboolean c = gtk_grid_get_column_homogeneous(grid);
            if (lua_isboolean(L, -2))
                c = lua_toboolean(L, -2);
            gboolean r = gtk_grid_get_row_homogeneous(grid);
            if (lua_isboolean(L, -1))
                r = lua_toboolean(L, -1);
            gtk_grid_set_column_homogeneous(grid, c);
            gtk_grid_set_row_homogeneous(grid, r);
        }
    }
    else if (g_strcmp0(field, "spacing") == 0)
    {
        if (lua_istable(L, 3))
        {
            lua_getfield(L, 3, "column");
            lua_getfield(L, 3, "row");
            gint c = luaL_optnumber(L, -2, gtk_grid_get_column_spacing(grid));
            gint r = luaL_optnumber(L, -1, gtk_grid_get_row_spacing(grid));
            if (c >= 0)
                gtk_grid_set_column_spacing(grid, c);
            if (r >= 0)
                gtk_grid_set_row_spacing(grid, r);
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
    {"__index",    index_gridL   },
    {"__newindex", newindex_gridL},
    {NULL,         NULL          }
};

int luaopen_gridL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, UDATA_GRIDL);
    luaL_setfuncs(L, gridLlib_m, 0);
    luaL_newlib(L, gridLlib_f, name);
    return 1;
}

