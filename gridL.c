#include "luaL.h"
#include "gridL.h"
#include "imageL.h"
#include "util.h"
#include "graph.h"

#define CHECK_ARGS \
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL); \
    gint left = luaL_checkint(L, 2); \
    gint top  = luaL_checkint(L, 3); \
    if (!grid_pos_exists(g, left, top)) \
        return 0; \
    gint idx = grid_index_by_pos(g, left, top); \
    thumb_t* thumb = (thumb_t*)g_ptr_array_index(g->thumbs, idx);


static thumb_t* thumb_new(void)
{
    thumb_t* thumb = g_new(thumb_t, 1);
    thumb->frame = (GtkFrame*)gtk_frame_new(NULL);
    thumb->image = (GtkImage*)gtk_image_new();
    gtk_container_add(GTK_CONTAINER(thumb->frame), (GtkWidget*)thumb->image);
    return thumb;
}

static void thumb_free(gpointer data)
{
    thumb_t* thumb = (thumb_t*)data;
    lua_pushlightuserdata(L, (void*)thumb);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    gtk_widget_destroy((GtkWidget*)thumb->image);
    gtk_widget_destroy((GtkWidget*)thumb->frame);
    g_free(thumb);
}


static inline gint grid_index_by_pos(gridL* g, gint left, gint top)
{
    return ((top - 1) * g->left + (left - 1));
}

static inline gboolean grid_pos_exists(gridL* g, gint left, gint top)
{
    return (left > 0 && top > 0 && left <= g->left && top <= g->top);
}


static int new_gridL(lua_State *L)
{
    gridL* g = (gridL*)lua_newuserdata(L, sizeof(gridL));

    g->grid  = (GtkGrid*)gtk_grid_new();
    g_object_ref(g->grid);
    gtk_grid_set_column_spacing(g->grid, 0);
    gtk_grid_set_row_spacing(g->grid, 0);
    gtk_grid_set_column_homogeneous(g->grid, TRUE);
    gtk_grid_set_row_homogeneous(g->grid, TRUE);

    g->left = 0;
    g->top  = 0;
    g->thumbs = g_ptr_array_new_with_free_func(&thumb_free);

    luaL_getmetatable(L, UDATA_GRIDL);
    lua_setmetatable(L, -2);
    return 1;
}

static int get_size_gridL(lua_State *L)
{
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    lua_pushnumber(L, g->left);
    lua_pushnumber(L, g->top);
    return 2;
}

static int set_size_gridL(lua_State *L)
{
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    gint left = luaL_checkint(L, 2);
    gint top  = luaL_checkint(L, 3);
    if (left < 0 || top < 0)
        return 0;
    if (left == 0 || top == 0)
    {
        left = 0;
        top  = 0;
    }
    if (left == g->left && top == g->top)
        return 0;
    g->left = left;
    g->top  = top;
    gint len = g->thumbs->len;
    g_ptr_array_set_size(g->thumbs, left * top);
    for (int t = 1; t <= top; t++)
        for (int l = 1; l <= left; l++)
        {
            gint idx = grid_index_by_pos(g, l, t);
            if (idx >= len)
                g_ptr_array_add(g->thumbs, thumb_new());
            GtkFrame* c = (GtkFrame*)g_ptr_array_index(g->thumbs, idx);
            GtkFrame* f = (GtkFrame*)gtk_grid_get_child_at(g->grid, l, t);
            if (f != c)
                gtk_grid_attach(g->grid, (GtkWidget*)c, l, t, 1, 1);
        }
    return 0;
}

static int get_name_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* name = gtk_widget_get_name((GtkWidget*)thumb->frame);
    lua_pushstring(L, name);
    return 1;
}

static int set_name_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* name = luaL_checkstring(L, 4);
    gtk_widget_set_name((GtkWidget*)thumb->frame, name);
    return 0;
}

static int get_label_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* label = gtk_frame_get_label(thumb->frame);
    if (label)
        lua_pushstring(L, label);
    return 1;
}

static int set_label_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* label = lua_tostring(L, 4);
    gtk_frame_set_label(thumb->frame, label);
    return 0;
}

static int get_gridL(lua_State *L)
{
    CHECK_ARGS;
    lua_pushlightuserdata(L, (void*)thumb);
    lua_gettable(L, LUA_REGISTRYINDEX);
    return 1;
}

static int set_gridL(lua_State *L)
{
    CHECK_ARGS;
    lua_pushlightuserdata(L, (void*)thumb);
    if (lua_isnil(L, 4))
        gtk_image_clear(thumb->image);
    else
    {
        imageL* i = (imageL*)luaL_checkudata(L, 4, UDATA_IMAGEL);
        gtk_image_set_from_pixbuf(thumb->image, i->pxb);
    }
    lua_pushvalue(L, 4);
    lua_settable(L, LUA_REGISTRYINDEX);
    /*gtk_widget_show_all((GtkWidget*)grid);*/
    return 0;
}

static int show_gridL(lua_State *L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    show_widget((GtkWidget*)g->grid);
    return 0;
}


static int gc_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    g_ptr_array_free(g->thumbs, TRUE);
    gtk_widget_destroy((GtkWidget*)g->grid);
    return 0;
}

static int index_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "spacing") == 0)
    {
        lua_newtable(L);
        lua_pushnumber(L, gtk_grid_get_column_spacing(g->grid));
        lua_setfield(L, -2, "column");
        lua_pushnumber(L, gtk_grid_get_row_spacing(g->grid));
        lua_setfield(L, -2, "row");
    }
    else if (g_strcmp0(field, "get_name") == 0)
        lua_pushcfunction(L, get_name_gridL);
    else if (g_strcmp0(field, "set_name") == 0)
        lua_pushcfunction(L, set_name_gridL);
    else if (g_strcmp0(field, "get_label") == 0)
        lua_pushcfunction(L, get_label_gridL);
    else if (g_strcmp0(field, "set_label") == 0)
        lua_pushcfunction(L, set_label_gridL);
    else if (g_strcmp0(field, "get") == 0)
        lua_pushcfunction(L, get_gridL);
    else if (g_strcmp0(field, "set") == 0)
        lua_pushcfunction(L, set_gridL);
    else if (g_strcmp0(field, "get_size") == 0)
        lua_pushcfunction(L, get_size_gridL);
    else if (g_strcmp0(field, "set_size") == 0)
        lua_pushcfunction(L, set_size_gridL);
    else if (g_strcmp0(field, "show") == 0)
        lua_pushcfunction(L, show_gridL);
    else
        lua_pushnil(L);
    return 1;
}

static int newindex_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "spacing") == 0)
    {
        if (lua_istable(L, 3))
        {
            lua_getfield(L, 3, "column");
            lua_getfield(L, 3, "row");
            if (lua_isnumber(L, -2))
            {
                gint c = lua_tonumber(L, -2);
                if (c >= 0)
                    gtk_grid_set_column_spacing(g->grid, c);
            }
            if (lua_isnumber(L, -1))
            {
                gint r = lua_tonumber(L, -1);
                if (r >= 0)
                    gtk_grid_set_column_spacing(g->grid, r);
            }
        }
    }
    else if (g_strcmp0(field, "name") == 0)
    {
        const gchar* name = luaL_checkstring(L, 3);
        gtk_widget_set_name((GtkWidget*)g->grid, name);
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

