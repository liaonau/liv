#include "luaL.h"
#include "gridL.h"
#include "imageL.h"
#include "util.h"
#include "idle.h"

#define CHECK_ARGS \
    gridL*  g   = (gridL*)luaL_checkudata(L, 1, GRID); \
    gint    idx = luaL_checkint(L, 2) - 1; \
    if (!grid_pos_exists(g, idx)) \
        return 0; \
    GtkFrame* frame = (GtkFrame*)g_ptr_array_index(g->frames, idx); \

static void frame_free(gpointer data)
{
    if (data)
    {
        g_object_unref(data);
        gtk_widget_destroy((GtkWidget*)data);
    }
}

static inline gboolean grid_pos_exists(gridL* g, gint index)
{
    return ((guint)index < g->frames->len);
}

static int new_gridL(lua_State* L)
{
    gridL* g = (gridL*)lua_newuserdata(L, sizeof(gridL));

    g->scroll = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
    g->grid   = (GtkGrid*)gtk_grid_new();
    g_object_ref(g->scroll);
    g_object_ref(g->grid);

    gtk_scrolled_window_set_policy(g->scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(g->scroll), (GtkWidget*)g->grid);

    gtk_grid_set_row_spacing(   g->grid, 0);
    gtk_grid_set_column_spacing(g->grid, 0);
    gtk_widget_set_valign((GtkWidget*)g->grid, GTK_ALIGN_CENTER);
    gtk_widget_set_halign((GtkWidget*)g->grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_homogeneous(   g->grid, FALSE);
    gtk_grid_set_column_homogeneous(g->grid, FALSE);

    g->rows  = 0;
    g->cols  = 0;
    g->size  = 1;
    g->frames = g_ptr_array_new_with_free_func(&frame_free);

    luaL_getmetatable(L, GRID);
    lua_setmetatable(L, -2);
    return 1;
}
static int get_size_gridL(lua_State* L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    lua_pushnumber(L, g->frames->len);
    lua_pushnumber(L, g->size);
    return 2;
}
static int set_size_gridL(lua_State* L)
{
    gridL*  g  = (gridL*)luaL_checkudata(L, 1, GRID);
    gint items = luaL_checkint(L, 2);
    gint size  = luaL_checkint(L, 3);
    if (items < 0 || size < 1)
        return 0;
    gint len = g->frames->len;
    g->size = size;
    g_ptr_array_set_size(g->frames, items);
    for (gint i = 0; i <items ; i++)
    {
        if (i >= len)
        {
            GtkWidget* frame = gtk_frame_new("");
            g_object_ref_sink(frame);

            GtkLabel* label = (GtkLabel*)gtk_frame_get_label_widget((GtkFrame*)frame);
            gtk_label_set_max_width_chars(label, 1);
            gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_START);
            gtk_label_set_line_wrap(label, FALSE);
            gtk_frame_set_shadow_type((GtkFrame*)frame, GTK_SHADOW_ETCHED_OUT);

            GtkWidget* image = gtk_image_new();
            gtk_container_add(GTK_CONTAINER(frame), image);
            gtk_widget_show_all(frame);

            g_ptr_array_add(g->frames, frame);
            g_ptr_array_remove_index_fast(g->frames, i);
        }
        GtkFrame* frame = (GtkFrame*)g_ptr_array_index(g->frames, i);
        GtkImage* image = (GtkImage*)gtk_bin_get_child(GTK_BIN(frame));
        gtk_widget_set_size_request((GtkWidget*)image, size, size);
    }
    return 0;
}
static int preferred_size_gridL(lua_State* L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    gint len = g->frames->len;
    gint w   = 1;
    gint h   = 1;
    for (gint i = 0; i < len; i++)
    {
        GtkFrame* f = (GtkFrame*)g_ptr_array_index(g->frames, i);
        GtkRequisition nat;
        gtk_widget_get_preferred_size((GtkWidget*)f, NULL, &nat);
        if (nat.width  > w)
            w = nat.width;
        if (nat.height > h)
            h = nat.height;
    }
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
}

static int get_name_gridL(lua_State* L)
{
    CHECK_ARGS;
    const gchar* name = gtk_widget_get_name((GtkWidget*)frame);
    lua_pushstring(L, name);
    return 1;
}
static int set_name_gridL(lua_State* L)
{
    CHECK_ARGS;
    const gchar* name = luaL_checkstring(L, 3);
    gtk_widget_set_name((GtkWidget*)frame, name);
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}
static int get_label_gridL(lua_State* L)
{
    CHECK_ARGS;
    const gchar* label = gtk_frame_get_label(frame);
    if (label)
        lua_pushstring(L, label);
    return 1;
}
static int set_label_gridL(lua_State* L)
{
    CHECK_ARGS;
    const gchar* label = lua_tostring(L, 3);
    gtk_frame_set_label(frame, label);
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}
static int clear_gridL(lua_State* L)
{
    CHECK_ARGS;
    GtkImage* image = (GtkImage*)gtk_bin_get_child(GTK_BIN(frame));
    gtk_image_clear(image);
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}
static int clear_all_gridL(lua_State* L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    gint len = g->frames->len;
    for (gint idx = 0; idx < len; idx++)
    {
        GtkFrame* frame = (GtkFrame*)g_ptr_array_index(g->frames, idx);
        GtkImage* image = (GtkImage*)gtk_bin_get_child(GTK_BIN(frame));
        gtk_image_clear(image);
    }
    gtk_widget_show_all((GtkWidget*)g->grid);
    return 0;
}
static int load_gridL(lua_State* L)
{
    CHECK_ARGS;
    imageL* i = (imageL*)luaL_checkudata(L, 3, IMAGE);
    GtkImage* image = (GtkImage*)gtk_bin_get_child(GTK_BIN(frame));
    gboolean show_deferred = lua_toboolean(L, 3);
    idle_load(L, image, i, show_deferred);
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}
static int detach_gridL(lua_State* L)
{
    CHECK_ARGS;
    gtk_container_remove(GTK_CONTAINER(g->grid), (GtkWidget*)frame);
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}
static int detach_all_gridL(lua_State* L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    gint len = g->frames->len;
    for (gint idx = 0; idx < len; idx++)
    {
        GtkFrame* frame = (GtkFrame*)g_ptr_array_index(g->frames, idx);
        gtk_container_remove(GTK_CONTAINER(g->grid), (GtkWidget*)frame);
    }
    gtk_widget_show_all((GtkWidget*)g->grid);
    return 0;
}
static int attach_gridL(lua_State* L)
{
    CHECK_ARGS;
    gint l = luaL_checkinteger(L, 3);
    gint t = luaL_checkinteger(L, 4);
    GtkWidget* c = gtk_grid_get_child_at(g->grid, l, t);
    if (frame != (GtkFrame*)c)
    {
        if (c)
            gtk_container_remove(GTK_CONTAINER(g->grid), c);
        gtk_grid_attach(g->grid, (GtkWidget*)frame, l, t, 1, 1);
    }
    gtk_widget_show_all((GtkWidget*)frame);
    return 0;
}

static int get_scroll_gridL(lua_State *L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)g->scroll);
    GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)g->scroll);
    gdouble hscroll = gtk_adjustment_get_value(hadj);
    lua_pushnumber(L, hscroll);
    gdouble vscroll = gtk_adjustment_get_value(vadj);
    lua_pushnumber(L, vscroll);
    gdouble min_hscroll = gtk_adjustment_get_lower(hadj);
    lua_pushnumber(L, min_hscroll);
    gdouble min_vscroll = gtk_adjustment_get_lower(vadj);
    lua_pushnumber(L, min_vscroll);
    gdouble max_hscroll = gtk_adjustment_get_upper(hadj);
    lua_pushnumber(L, max_hscroll);
    gdouble max_vscroll = gtk_adjustment_get_upper(vadj);
    lua_pushnumber(L, max_vscroll);
    return 6;
}
static int set_scroll_gridL(lua_State *L)
{
    gridL* g = (gridL*)luaL_checkudata(L, 1, GRID);
    if (lua_istable(L, 2))
    {
        GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)g->scroll);
        GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)g->scroll);
        gdouble hscroll = gtk_adjustment_get_value(hadj);
        gdouble vscroll = gtk_adjustment_get_value(vadj);
        lua_getfield(L, 2, "h");
        gdouble h = luaL_optnumber(L, -1, gtk_adjustment_get_value(hadj));
        lua_getfield(L, 2, "v");
        gdouble v = luaL_optnumber(L, -1, gtk_adjustment_get_value(vadj));
        if (h != hscroll)
            gtk_adjustment_set_value(hadj, h);
        if (v != vscroll)
            gtk_adjustment_set_value(vadj, v);
    }
    return 0;
}

static int gc_gridL(lua_State* L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, GRID);
    g_ptr_array_free(g->frames, TRUE);
    g_object_unref(g->grid);
    g_object_unref(g->scroll);
    gtk_widget_destroy((GtkWidget*)g->grid);
    gtk_widget_destroy((GtkWidget*)g->scroll);
    return 0;
}
static int tostring_gridL(lua_State* L)
{
    luaL_checkudata(L, 1, GRID);
    lua_pushstring(L, LIB_GRIDL);
    return 1;
}
static int index_gridL(lua_State* L)
{
    gridL *g = (gridL*)luaL_checkudata(L, 1, GRID);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_FUNC(L, field, get_size,       grid);
    CASE_FUNC(L, field, set_size,       grid);
    CASE_FUNC(L, field, preferred_size, grid);

    CASE_FUNC(L, field, get_name,       grid);
    CASE_FUNC(L, field, set_name,       grid);
    CASE_FUNC(L, field, get_label,      grid);
    CASE_FUNC(L, field, set_label,      grid);

    CASE_FUNC(L, field, get_scroll,     grid);
    CASE_FUNC(L, field, set_scroll,     grid);

    CASE_FUNC(L, field, clear,          grid);
    CASE_FUNC(L, field, clear_all,      grid);
    CASE_FUNC(L, field, load,           grid);
    CASE_FUNC(L, field, detach,         grid);
    CASE_FUNC(L, field, detach_all,     grid);
    CASE_FUNC(L, field, attach,         grid);

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
int luaopen_gridL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, GRID);
    luaL_setfuncs(L, gridLlib_m, 0);
    luaL_newlib(L, gridLlib_f, name);
    return 1;
}
