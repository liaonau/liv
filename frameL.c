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
#include "frameL.h"
#include "imageL.h"
#include "util.h"
#include "task.h"
#include "inlined.h"

static int new_frameL(lua_State *L)
{
    frameL* f = (frameL*)lua_newuserdata(L, sizeof(frameL));

    f->frame = (GtkFrame*)gtk_frame_new(NULL);
    g_object_ref_sink(f->frame);

    gtk_frame_set_shadow_type((GtkFrame*)f->frame, GTK_SHADOW_NONE);

    f->image = (GtkImage*)gtk_image_new();
    gtk_container_add(GTK_CONTAINER(f->frame), (GtkWidget*)f->image);

    g_rw_lock_init(&f->lock);
    f->time = 0;
    f->ref  = LUA_REFNIL;

    gtk_widget_show_all((GtkWidget*)f->frame);

    luaL_getmetatable(L, FRAME);
    lua_setmetatable(L, -2);
    return 1;
}

static void frame_update_from_pixbuf(frameL* f, GdkPixbuf* pxb)
{
    if (!pxb)
        gtk_image_set_from_pixbuf(f->image, BROKENpxb);
    else
    {
        gtk_image_set_from_pixbuf(f->image, pxb);
        g_object_unref(pxb);
    }
    gtk_widget_show_all((GtkWidget*)f->frame);
}
static void luaH_frame_update(lua_State* L, frameL* f)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, f->ref);
    if (lua_isuserdata(L, -1))
    {
        imageL* i = (imageL*)luaL_checkudata(L, -1, IMAGE);
        gulong time;
        g_rw_lock_writer_lock(&f->lock);
        {
            f->time++;
            time = f->time;
        }
        g_rw_lock_writer_unlock(&f->lock);
        if (!i->pxb)
            gtk_window_set_icon(window, LOADINGpxb);
            /*gtk_image_set_from_animation(f->image, DEFERREDpxb);*/
        task_frame_from_image_pixbuf(f, i, time, &frame_update_from_pixbuf);
    }
    lua_pop(L, 1);
}
static void frame_image_changed_cb(GObject* emitter, gpointer lua_state_data, gpointer self)
{
    lua_State* L = (lua_State*) lua_state_data;
    frameL*    f = (frameL*)self;
    luaH_frame_update(L, f);
}

static int image_get_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    lua_rawgeti(L, LUA_REGISTRYINDEX, f->ref);
    return 1;
}
static int image_set_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);

    lua_rawgeti(L, LUA_REGISTRYINDEX, f->ref);
    if (lua_isuserdata(L, -1))
    {
        imageL* i = (imageL*)luaL_checkudata(L, -1, IMAGE);
        g_signal_handler_disconnect(i->emitter, f->handler);
    }
    lua_pop(L, 1);

    luaL_unref(L, LUA_REGISTRYINDEX, f->ref);
    if (lua_isuserdata(L, 2))
    {
        imageL* i = (imageL*)luaL_checkudata(L, 2, IMAGE);
        lua_pushvalue(L, 2);
        f->ref     = luaL_ref(L, LUA_REGISTRYINDEX);
        f->handler = g_signal_connect(i->emitter, IMAGE_CHANGED_SIGNAL, (GCallback)frame_image_changed_cb, (gpointer)f);
    }
    else
        f->ref = LUA_REFNIL;

    luaH_frame_update(L, f);
    return 0;
}

static int name_get_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* name = gtk_widget_get_name((GtkWidget*)f->frame);
    if (name)
        lua_pushstring(L, name);
    return 1;
}
static int name_set_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* name = luaL_checkstring(L, 2);
    gtk_widget_set_name((GtkWidget*)f->frame, name);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}
static int label_get_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* label = gtk_frame_get_label(f->frame);
    if (label)
        lua_pushstring(L, label);
    return 1;
}
static int label_set_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* text = lua_tostring(L, 2);
    if (text)
    {
        gtk_frame_set_label(f->frame, "");
        GtkLabel* label = (GtkLabel*)gtk_frame_get_label_widget(f->frame);
        gtk_label_set_markup(label, text);
    }
    else
        gtk_frame_set_label(f->frame, text);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}

static int clear_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    gtk_image_clear(f->image);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}
static int size_request_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    gint w = luaL_checkint(L, 2);
    gint h = luaL_checkint(L, 3);
    if (w < 1 || h < 1)
    {
        w = -1;
        h = -1;
    }
    gtk_widget_set_size_request((GtkWidget*)f->image, w, h);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}
static int preferred_size_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    GtkRequisition rq;
    gtk_widget_get_preferred_size((GtkWidget*)f->frame, NULL, &rq);
    gint nw = rq.width;
    gint nh = rq.height;
    gtk_widget_get_preferred_size((GtkWidget*)f->image, NULL, &rq);
    gint rw = rq.width;
    gint rh = rq.height;
    gint iw = rw;
    gint ih = rh;
    lua_rawgeti(L, LUA_REGISTRYINDEX, f->ref);
    if (lua_isuserdata(L, -1))
    {
        imageL* i = (imageL*)luaL_checkudata(L, -1, IMAGE);
        iw = i->width;
        ih = i->height;
    }
    lua_pop(L, 1);
    gint w = nw - rw + iw;
    gint h = nh - rh + ih;
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
}
static int class_add_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* class = luaL_checkstring(L, 2);
    GtkStyleContext* context = gtk_widget_get_style_context((GtkWidget*)f->frame);
    gtk_style_context_add_class(context, class);
    return 0;
}
static int class_remove_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* class = luaL_checkstring(L, 2);
    GtkStyleContext* context = gtk_widget_get_style_context((GtkWidget*)f->frame);
    gtk_style_context_remove_class(context, class);
    return 0;
}

static int gc_frameL(lua_State *L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    luaL_unref(L, LUA_REGISTRYINDEX, f->ref);
    g_object_unref(f->frame);
    g_rw_lock_clear(&f->lock);
    return 0;
}
static int tostring_frameL(lua_State *L)
{
    luaL_checkudata(L, 1, FRAME);
    lua_pushstring(L, FRAME);
    return 1;
}
static int index_frameL(lua_State *L)
{
    luaL_checkudata(L, 1, FRAME);
    const gchar* field = luaL_checkstring(L, 2);

    INDEX_FIELD(image,  frame);

    INDEX_FIELD(name,   frame);
    INDEX_FIELD(label,  frame);

    CASE_FUNC(clear,          frame);
    CASE_FUNC(size_request,   frame);
    CASE_FUNC(preferred_size, frame);
    CASE_FUNC(class_add,      frame);
    CASE_FUNC(class_remove,   frame);

    return 1;
}
static int newindex_frameL(lua_State *L)
{
    luaL_checkudata(L, 1, FRAME);
    const gchar* field = luaL_checkstring(L, 2);

    NEWINDEX_FIELD(image,  frame);

    NEWINDEX_FIELD(name,   frame);
    NEWINDEX_FIELD(label,  frame);

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
    {"__tostring", tostring_frameL},
    {NULL,         NULL           }
};
int luaopen_frameL(lua_State *L)
{
    luaL_newmetatable(L, FRAME);
    luaL_setfuncs(L, frameLlib_m, 0);
    luaL_newlib(L, frameLlib_f, FRAME);
    return 1;
}
