#include "luaL.h"
#include "gridL.h"
#include "imageL.h"
#include "util.h"

#define CHECK_ARGS \
    gridL*  g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL); \
    gint left = luaL_checkint(L, 2); \
    gint top  = luaL_checkint(L, 3); \
    if (!grid_pos_exists(g, left, top)) \
        return 0; \
    gint idx = grid_index_by_pos(g, left, top); \
    GtkFrame* frame = (GtkFrame*)g_ptr_array_index(g->frames, idx); \

static void frame_free(gpointer data)
{
    GtkFrame*  frame = (GtkFrame*)data;
    GtkWidget* image = gtk_bin_get_child(frame);
    gtk_widget_destroy((GtkWidget*)frame);
}

static inline gint grid_index_by_pos(gridL* g, gint left, gint top)
{
    return ((top - 1) * g->rows + (left - 1));
}
static inline gboolean grid_pos_exists(gridL* g, gint left, gint top)
{
    return (left > 0 && top > 0 && left <= g->rows && top <= g->cols);
}

static int new_gridL(lua_State *L)
{
    gridL* g = (gridL*)lua_newuserdata(L, sizeof(gridL));

    g->grid  = (GtkGrid*)gtk_grid_new();
    g_object_ref(g->grid);
    gtk_grid_set_row_spacing(   g->grid, 0);
    gtk_grid_set_column_spacing(g->grid, 0);
    gtk_grid_set_row_homogeneous(   g->grid, TRUE);
    gtk_grid_set_column_homogeneous(g->grid, TRUE);

    g->rows = 0;
    g->cols = 0;
    g->frames = g_ptr_array_new_with_free_func(&frame_free);

    luaL_getmetatable(L, UDATA_GRIDL);
    lua_setmetatable(L, -2);
    return 1;
}
static int get_size_gridL(lua_State *L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    lua_pushnumber(L, g->rows);
    lua_pushnumber(L, g->cols);
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
    if (left == g->rows && top == g->cols)
        return 0;
    g->rows = left;
    g->cols  = top;
    gint len = g->frames->len;
    g_ptr_array_set_size(g->frames, left * top);
    for (gint t = 1; t <= top; t++)
        for (gint l = 1; l <= left; l++)
        {
            gint idx = grid_index_by_pos(g, l, t);
            if (idx >= len)
            {
                GtkFrame* n = gtk_frame_new(NULL);
                GtkImage* i = gtk_image_new();
            }
            GtkFrame* f = (GtkFrame*)g_ptr_array_index(g->frames, idx);
            GtkFrame* c = (GtkFrame*)gtk_grid_get_child_at(g->grid, l, t);
            if (f != c)
            {
                if (c)
                    gtk_container_remove(GTK_CONTAINER(g->grid), (GtkWidget*)c);
                gtk_grid_attach(g->grid, (GtkWidget*)f, l, t, 1, 1);
            }
        }
    gtk_widget_show_all((GtkWidget*)g->grid);
    return 0;
}
static int get_name_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* name = gtk_widget_get_name((GtkWidget*)frame);
    lua_pushstring(L, name);
    return 1;
}
static int set_name_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* name = luaL_checkstring(L, 4);
    gtk_widget_set_name((GtkWidget*)frame, name);
    return 0;
}
static int get_label_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* label = gtk_frame_get_label(frame);
    if (label)
        lua_pushstring(L, label);
    return 1;
}
static int set_label_gridL(lua_State *L)
{
    CHECK_ARGS;
    const gchar* label = lua_tostring(L, 4);
    gtk_frame_set_label(frame, label);
    return 0;
}
static int clear_gridL(lua_State *L)
{
    CHECK_ARGS;
    GtkImage* image = (GtkImage*)gtk_bin_get_child(frame);
    gtk_image_clear(image);
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}
static int load_gridL(lua_State *L)
{
    CHECK_ARGS;
    imageL* i = (imageL*)luaL_checkudata(L, 4, UDATA_IMAGEL);
    GtkImage* image = (GtkImage*)gtk_bin_get_child(frame);
    GdkPixbuf* pxb = image_create_pixbuf(i);
    gtk_image_set_from_pixbuf(image, pxb);
    g_object_unref(pxb);
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}

static int gc_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    g_ptr_array_free(g->frames, TRUE);
    g_object_unref(g->grid);
    gtk_widget_destroy((GtkWidget*)g->grid);
    return 0;
}
static int index_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_FUNC(L, field, "get",       UDATA_GRIDL);
    CASE_FUNC(L, field, "set",       UDATA_GRIDL);
    CASE_FUNC(L, field, "get_name",  UDATA_GRIDL);
    CASE_FUNC(L, field, "set_name",  UDATA_GRIDL);
    CASE_FUNC(L, field, "get_size",  UDATA_GRIDL);
    CASE_FUNC(L, field, "set_size",  UDATA_GRIDL);
    CASE_FUNC(L, field, "get_label", UDATA_GRIDL);
    CASE_FUNC(L, field, "set_label", UDATA_GRIDL);
    CASE_FUNC(L, field, "load",      UDATA_GRIDL);
    CASE_FUNC(L, field, "clear",     UDATA_GRIDL);
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
static int newindex_gridL(lua_State *L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, UDATA_GRIDL);
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
                    gtk_grid_set_row_spacing(g->grid, r);
            }
            lua_getfield(L, 3, "column");
            if (lua_isnumber(L, -1))
            {
                gint c = lua_tonumber(L, -1);
                if (c >= 0)
                    gtk_grid_set_column_spacing(g->grid, c);
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
