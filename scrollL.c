#include "luaL.h"
#include "scrollL.h"
#include "imageL.h"
#include "util.h"
#include "graph.h"

static int new_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)lua_newuserdata(L, sizeof(scrollL));
    s->scroll = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
    g_object_ref(s->scroll);

    gtk_scrolled_window_set_policy(s->scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    luaL_getmetatable(L, UDATA_SCROLLL);
    lua_setmetatable(L, -2);
    return 1;
}

static int show_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)luaL_checkudata(L, 1, UDATA_SCROLLL);
    show_widget((GtkWidget*)s->scroll);
    return 0;
}

static int adjustment_scrollL(lua_State *L)
{
    scrollL* s = (scrollL*)luaL_checkudata(L, 1, UDATA_SCROLLL);
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


static int gc_scrollL(lua_State *L)
{
    scrollL *s = (scrollL*)luaL_checkudata(L, 1, UDATA_SCROLLL);

    lua_pushlightuserdata(L, (void*)s);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    gtk_widget_destroy((GtkWidget*)s->scroll);
    return 0;
}

static int index_scrollL(lua_State *L)
{
    scrollL *s = (scrollL*)luaL_checkudata(L, 1, UDATA_SCROLLL);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "image") == 0)
    {
        lua_pushlightuserdata(L, (void*)s);
        lua_gettable(L, LUA_REGISTRYINDEX);
    }
    else if (g_strcmp0(field, "adjustment") == 0)
        lua_pushcfunction(L, adjustment_scrollL);
    else if (g_strcmp0(field, "show") == 0)
        lua_pushcfunction(L, show_scrollL);
    else
        lua_pushnil(L);
    return 1;
}

static int newindex_scrollL(lua_State *L)
{
    scrollL *s = (scrollL*)luaL_checkudata(L, 1, UDATA_SCROLLL);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "image") == 0)
    {
        gtk_container_foreach(GTK_CONTAINER(s->scroll), (GtkCallback)gtk_container_remove, s->scroll);
        if (!lua_isnil(L, 3))
        {
            imageL* i = (imageL*)luaL_checkudata(L, 3, UDATA_IMAGEL);
            gtk_container_add(GTK_CONTAINER(s->scroll), (GtkWidget*)i->image);
        }
        lua_pushlightuserdata(L, (void*)s);
        lua_pushvalue(L, 3);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
    return 0;
}

static const struct luaL_Reg scrollLlib_f [] =
{
    {"new", new_scrollL},
    {NULL,  NULL       }
};

static const struct luaL_Reg scrollLlib_m [] =
{
    {"__gc",       gc_scrollL      },
    {"__index",    index_scrollL   },
    {"__newindex", newindex_scrollL},
    {NULL,         NULL            }
};

int luaopen_scrollL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, UDATA_SCROLLL);
    luaL_setfuncs(L, scrollLlib_m, 0);
    luaL_newlib(L, scrollLlib_f, name);
    return 1;
}

