#include "luaL.h"
#include "gridL.h"
#include "imageL.h"
#include "util.h"

static void clear_cb(GtkWidget* widget, gpointer self)
{
    gtk_container_remove(GTK_CONTAINER(self), widget);
}

static void grid_clear_cb(GtkWidget* widget, gpointer data)
{
    lua_State* L = (lua_State*)data;
    gridL* self = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    gtk_container_remove(GTK_CONTAINER(self->grid), widget);
    lua_pushlightuserdata(L, (void*)widget);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
}

static void grid_count_cb(GtkWidget* widget, gpointer data)
{
    grid_count_t* gc = (grid_count_t*)data;
    GValue vall = G_VALUE_INIT;
    GValue valt = G_VALUE_INIT;
    g_value_init(&vall, G_TYPE_INT);
    g_value_init(&valt, G_TYPE_INT);
    gtk_container_child_get_property(GTK_CONTAINER(gc->grid), widget, "left-attach", &vall);
    gtk_container_child_get_property(GTK_CONTAINER(gc->grid), widget, "top-attach",  &valt);
    gint l = g_value_get_int(&vall);
    gint t = g_value_get_int(&valt);
    if (gc->left < l)
        gc->left = l;
    if (gc->top  < t)
        gc->top  = t;
}


static int new_gridL(lua_State *L)
{
    gridL* g = (gridL*)lua_newuserdata(L, sizeof(gridL));
    g->grid  = (GtkGrid*)gtk_grid_new();
    if (lua_isstring(L, 1))
    {
        const gchar* name = lua_tostring(L, 1);
        gtk_widget_set_name((GtkWidget*)g->grid, name);
        lua_remove(L, 1);
    }
    g_object_ref(g->grid);
    gtk_grid_set_column_spacing(g->grid, 10);
    gtk_grid_set_row_spacing(g->grid, 10);
    gtk_grid_set_column_homogeneous(g->grid, TRUE);
    gtk_grid_set_row_homogeneous(g->grid, TRUE);

    luaL_getmetatable(L, UDATA_GRIDL);
    lua_setmetatable(L, -2);
    return 1;
}

static int clear_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    GtkGrid* grid = g->grid;
    gtk_container_foreach(GTK_CONTAINER(grid), grid_clear_cb, (void*)L);
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
    GtkFrame* frame = i->frame;
    gtk_grid_attach(grid, (GtkWidget*)frame, left, top, 1, 1);
    lua_pushlightuserdata(L, (void*)i->frame);
    lua_pushvalue(L, 2);
    lua_settable(L, LUA_REGISTRYINDEX);

    gtk_widget_show_all((GtkWidget*)grid);
    return 0;
}

static int child_gridL(lua_State *L)
{
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    gint left = luaL_checknumber(L, 2);
    gint top  = luaL_checknumber(L, 3);
    GtkGrid* grid   = g->grid;
    GtkFrame* frame = (GtkFrame*)gtk_grid_get_child_at(grid, left, top);
    if (frame)
    {
        lua_pushlightuserdata(L, (void*)frame);
        lua_gettable(L, LUA_REGISTRYINDEX);
        luaL_checkudata(L, -1, UDATA_IMAGEL);
        return 1;
    }
    return 0;
}

static int show_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    GtkGrid* grid = g->grid;
    gtk_container_foreach(GTK_CONTAINER(scroll), clear_cb, scroll);
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
    if (g_strcmp0(field, "spacing") == 0)
    {
        lua_newtable(L);
        lua_pushnumber(L, gtk_grid_get_column_spacing(grid));
        lua_setfield(L, -2, "column");
        lua_pushnumber(L, gtk_grid_get_row_spacing(grid));
        lua_setfield(L, -2, "row");
    }
    else if (g_strcmp0(field, "size") == 0)
    {
        grid_count_t gc;
        gc.grid = grid;
        gc.left = 0;
        gc.top  = 0;
        gtk_container_foreach(GTK_CONTAINER(grid), grid_count_cb, (void*)&gc);
        lua_newtable(L);
        lua_pushnumber(L, gc.left);
        lua_setfield(L, -2, "left");
        lua_pushnumber(L, gc.top);
        lua_setfield(L, -2, "top");
    }
    else if (g_strcmp0(field, "child") == 0)
        lua_pushcfunction(L, child_gridL);
    else if (g_strcmp0(field, "clear") == 0)
        lua_pushcfunction(L, clear_gridL);
    else if (g_strcmp0(field, "attach") == 0)
        lua_pushcfunction(L, attach_gridL);
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
    if (g_strcmp0(field, "spacing") == 0)
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

