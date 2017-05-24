#include "luaL.h"
include "scrollL.h"
#include "imageL.h"
#include "util.h"
#include "idle.h"

static int new_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)lua_newuserdata(L, sizeof(scrollL));

    s->scroll = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
    s->image  = (GtkImage*)gtk_image_new();
    g_object_ref(s->scroll);
    gtk_container_add(GTK_CONTAINER(s->scroll), (GtkWidget*)s->image);
    gtk_scrolled_window_set_policy(s->scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    luaL_getmetatable(L, SCROLL);
    lua_setmetatable(L, -2);
    return 1;
}
static int get_scroll_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)luaL_checkudata(L, 1, SCROLL);
    GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)s->scroll);
    GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)s->scroll);
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
static int set_scroll_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)luaL_checkudata(L, 1, SCROLL);
    if (lua_istable(L, 2))
    {
        GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)s->scroll);
        GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)s->scroll);
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

static int load_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)luaL_checkudata(L, 1, SCROLL);
    imageL*  i = (imageL*)luaL_checkudata(L, 2, IMAGE);
    gboolean show_deferred = lua_toboolean(L, 3);
    idle_load(L, s->image, i, show_deferred);
    return 0;
}
static int clear_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)luaL_checkudata(L, 1, SCROLL);
    gtk_image_clear(s->image);
    gtk_widget_show((GtkWidget*)s->image);
    return 0;
}

static int gc_scrollL(lua_State *L)
{
    scrollL *s = (scrollL*)luaL_checkudata(L, 1, SCROLL);
    g_object_unref(s->scroll);
    gtk_widget_destroy((GtkWidget*)s->scroll);
    return 0;
}
static int tostring_scrollL(lua_State *L)
{
    luaL_checkudata(L, 1, SCROLL);
    lua_pushstring(L, LIB_SCROLLL);
    return 1;
}
static int index_scrollL(lua_State *L)
{
    /*scrollL *s = (scrollL*)luaL_checkudata(L, 1, SCROLL);*/
    luaL_checkudata(L, 1, SCROLL);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_FUNC(L, field, get_scroll, scroll);
    CASE_FUNC(L, field, set_scroll, scroll);
    CASE_FUNC(L, field, load,       scroll);
    CASE_FUNC(L, field, clear,      scroll);
    return 1;
}

static const struct luaL_Reg scrollLlib_f [] =
{
    {"new", new_scrollL},
    {NULL,  NULL       }
};
static const struct luaL_Reg scrollLlib_m [] =
{
    {"__gc",       gc_scrollL      },
    {"__tostring", tostring_scrollL},
    {"__index",    index_scrollL   },
    {NULL,         NULL            }
};
int luaopen_scrollL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, SCROLL);
    luaL_setfuncs(L, scrollLlib_m, 0);
    luaL_newlib(L, scrollLlib_f, name);
    return 1;
}
