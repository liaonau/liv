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
    return 0;
}

static int add_gridL(lua_State *L)
{
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    imageL* i = (imageL*)luaL_checkudata(L, 2, UDATA_IMAGEL);
    GtkGrid* grid   = g->grid;
    GtkImage* image = i->image;
    gtk_container_add(GTK_CONTAINER(grid), (GtkWidget*)image);
    return 0;
}

static int show_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    GtkGrid* grid = g->grid;
    gtk_container_foreach(GTK_CONTAINER(swindow), container_clear_cb, swindow);
    gtk_container_add(GTK_CONTAINER(swindow), (GtkWidget*)grid);
    gtk_widget_show((GtkWidget*)grid);
    return 0;
}

static int gc_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    g_object_unref(g->grid);
    return 0;
}

static const struct luaL_Reg gridLlib_f [] =
{
    {"new", new_gridL},
    {NULL,  NULL}
};

static const struct luaL_Reg gridLlib_m [] =
{
    {"clear",  clear_gridL },
    {"attach", attach_gridL},
    {"add",    add_gridL   },
    {"show",   show_gridL  },
    {NULL,     NULL        }
};

static const struct luaL_Reg gridLlib_gc [] =
{
    {"__gc",  gc_gridL},
    {NULL,     NULL   }
};

int luaopen_gridL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, UDATA_GRIDL);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, gridLlib_m, 0);
    luaL_setfuncs(L, gridLlib_gc, 0);
    luaL_newlib(L, gridLlib_f, name);
    return 1;
}

