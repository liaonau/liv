#include "luaL.h"
#include "frameL.h"
#include "imageL.h"
#include "util.h"
#include "graph.h"

static int new_frameL(lua_State *L)
{
    frameL* f = (frameL*)lua_newuserdata(L, sizeof(frameL));
    f->scroll = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
    f->image  = (GtkImage*)gtk_image_new();
    gtk_container_add(GTK_CONTAINER(f->scroll), (GtkWidget*)f->image);
    g_object_ref(f->scroll);
    g_object_ref(f->image);

    gtk_scrolled_window_set_policy(f->scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    luaL_getmetatable(L, UDATA_FRAMEL);
    lua_setmetatable(L, -2);
    return 1;
}

static int show_frameL(lua_State *L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, UDATA_FRAMEL);
    show_widget((GtkWidget*)f->scroll);
    return 0;
}


static int gc_frameL(lua_State *L)
{
    frameL *f = (frameL*)luaL_checkudata(L, 1, UDATA_FRAMEL);
    /*gtk_image_clear(f->image);*/

    lua_pushlightuserdata(L, (void*)f);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    gtk_widget_destroy((GtkWidget*)f->scroll);
    gtk_widget_destroy((GtkWidget*)f->image);
    return 0;
}

static int index_frameL(lua_State *L)
{
    frameL *f = (frameL*)luaL_checkudata(L, 1, UDATA_FRAMEL);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "image") == 0)
    {
        lua_pushlightuserdata(L, (void*)f);
        lua_gettable(L, LUA_REGISTRYINDEX);
    }
    /*else if (g_strcmp0(field, "show") == 0)*/
        /*lua_pushcfunction(L, show_frameL);*/
    else if (g_strcmp0(field, "width") == 0)
    {
        GtkAllocation alloc;
        gtk_widget_get_allocation((GtkWidget*)f->scroll, &alloc);
        lua_pushnumber(L, alloc.width);
    }
    else if (g_strcmp0(field, "height") == 0)
    {
        GtkAllocation alloc;
        gtk_widget_get_allocation((GtkWidget*)f->scroll, &alloc);
        lua_pushnumber(L, alloc.height);
    }
    else if (g_strcmp0(field, "hscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)f->scroll);
        gdouble value = gtk_adjustment_get_value(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "vscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)f->scroll);
        gdouble value = gtk_adjustment_get_value(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "max_hscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)f->scroll);
        gdouble value = gtk_adjustment_get_upper(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "max_vscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)f->scroll);
        gdouble value = gtk_adjustment_get_upper(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "min_hscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)f->scroll);
        gdouble value = gtk_adjustment_get_lower(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "max_vscroll") == 0)
    {
        GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)f->scroll);
        gdouble value = gtk_adjustment_get_lower(adj);
        lua_pushnumber(L, value);
    }
    else if (g_strcmp0(field, "show") == 0)
        lua_pushcfunction(L, show_frameL);
    else
        lua_pushnil(L);
    return 1;
}

static int newindex_frameL(lua_State *L)
{
    frameL *f = (frameL*)luaL_checkudata(L, 1, UDATA_FRAMEL);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "image") == 0)
    {
        lua_pushlightuserdata(L, (void*)f);
        if (lua_isnil(L, 3))
            gtk_image_clear(f->image);
        else
        {
            imageL* i = (imageL*)luaL_checkudata(L, 3, UDATA_IMAGEL);
            gtk_image_set_from_pixbuf(f->image, i->pxb);
        }
        lua_pushvalue(L, 3);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
    return 0;
}

static const struct luaL_Reg frameLlib_f [] =
{
    {"new", new_frameL},
    {NULL,  NULL      }
};

static const struct luaL_Reg frameLlib_m [] =
{
    {"__gc",       gc_frameL      },
    {"__index",    index_frameL   },
    {"__newindex", newindex_frameL},
    {NULL,         NULL           }
};

int luaopen_frameL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, UDATA_FRAMEL);
    luaL_setfuncs(L, frameLlib_m, 0);
    luaL_newlib(L, frameLlib_f, name);
    return 1;
}

