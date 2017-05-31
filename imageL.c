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
#include "util.h"
#include "imageL.h"
#include "inlined.h"
#include "task.h"

//таблица умножения группы четырех вращений и вертикального и горизонтального отражения
//a0-a3 — вращения, a4,a6 — горизонтальные и вертикальные отражения
static const guint8 states[8][8] =
{
    {0, 1, 2, 3, 4, 5, 6, 7},
    {1, 2, 3, 0, 7, 4, 5, 6},
    {2, 3, 0, 1, 6, 7, 4, 5},
    {3, 0, 1, 2, 5, 6, 7, 4},
    {4, 5, 6, 7, 0, 1, 2, 3},
    {5, 6, 7, 4, 4, 0, 1, 2},
    {6, 7, 4, 5, 2, 3, 0, 1},
    {7, 4, 5, 3, 1, 2, 3, 1}
};

static inline gboolean image_is_swapped(imageL* i)
{
    return (i->state % 2);
}
static inline gboolean image_is_broken(imageL* i)
{
    return (i->format == NULL);
}
static inline gboolean image_is_memorized(imageL* i)
{
    return (GDK_IS_PIXBUF(i->pxb));
}
static inline gboolean image_is_writable(imageL* i)
{
    return (!image_is_broken(i) && gdk_pixbuf_format_is_writable(i->format));
}
static inline const gchar* image_format_name(imageL* i)
{
    return (image_is_broken(i) ? NULL : gdk_pixbuf_format_get_name(i->format));
}

static void image_register_signal(void)
{
    g_signal_new("image-changed",
            G_TYPE_OBJECT,
            G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
            g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE, 1, G_TYPE_POINTER);
    /*info("signal created");*/
}
static void image_emit_signal(imageL* i, lua_State* L)
{
    /*info("%s emitting signal", i->path);*/
    g_signal_emit_by_name(i->emitter, IMAGE_CHANGED_SIGNAL, (gpointer)L);
}

GdkPixbuf* image_get_scaled_pixbuf(imageL* i, gint width, gint height)
{
    if (image_is_broken(i))
        return NULL;
    GdkPixbuf* pxb;
    gboolean unscaled = (width == i->native_width && height == i->native_height);
    if (image_is_memorized(i))
    {
        if (unscaled)
        {
            pxb = i->pxb;
            g_object_ref(pxb);
        }
        else
            pxb = gdk_pixbuf_scale_simple(i->pxb, width, height, GDK_INTERP_BILINEAR);
    }
    else
        pxb = gdk_pixbuf_new_from_file_at_scale(i->path, width, height, FALSE, NULL);
    return pxb;
}
GdkPixbuf* image_get_stated_pixbuf(imageL* i, GdkPixbuf* startpxb, guint8 state)
{
    if (startpxb == NULL)
        return NULL;
    GdkPixbuf* pxb = NULL;
    GdkPixbuf* tmppxb;
    switch (state)
    {
    case 0:
        pxb = startpxb;
        g_object_ref(pxb);
        break;
    case 1:
        pxb = gdk_pixbuf_rotate_simple(startpxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        break;
    case 2:
        pxb = gdk_pixbuf_rotate_simple(startpxb, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
        break;
    case 3:
        pxb = gdk_pixbuf_rotate_simple(startpxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        break;
    case 4:
        pxb = gdk_pixbuf_flip(startpxb, TRUE);
        break;
    case 5:
        tmppxb = gdk_pixbuf_rotate_simple(startpxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        pxb = gdk_pixbuf_flip(tmppxb, TRUE);
        g_object_unref(tmppxb);
        break;
    case 6:
        pxb = gdk_pixbuf_flip(startpxb, FALSE);
        break;
    case 7:
        tmppxb = gdk_pixbuf_rotate_simple(startpxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        pxb = gdk_pixbuf_flip(tmppxb, TRUE);
        g_object_unref(tmppxb);
        break;
    }
    g_object_unref(startpxb);
    return pxb;
}
/*не потокобезопасно*/
GdkPixbuf* image_get_current_pixbuf(imageL* i)
{
    GdkPixbuf* scaledpxb;
    GdkPixbuf* statedpxb;

    scaledpxb = image_get_scaled_pixbuf(i, i->width, i->height);
    statedpxb = image_get_stated_pixbuf(i, scaledpxb, i->state);
    return statedpxb;
}

static int info_imageL(lua_State *L)
{
    const gchar* path = luaL_checkstring(L, 1);
    gint w, h;
    GdkPixbufFormat* format = gdk_pixbuf_get_file_info(path, &w, &h);
    lua_pushboolean(L, (format != NULL));
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    if (format != NULL)
        lua_pushstring(L, gdk_pixbuf_format_get_name(format));
    else
        lua_pushnil(L);
    return 4;
}
static int new_imageL(lua_State *L)
{
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));
    const gchar* path = luaL_checkstring(L, 1);
    gboolean memorize = lua_toboolean(L, 2);
    i->path    = g_strdup(path);
    i->emitter = g_object_newv(G_TYPE_OBJECT, 0, NULL);
    i->pxb     = NULL;
    i->state   = 0;
    i->format  = gdk_pixbuf_get_file_info(i->path, &i->native_width, &i->native_height);
    if (image_is_broken(i))
    {
        i->native_width  = 1;
        i->native_height = 1;
    }
    else
    {
        if (lua_isnumber(L, 3))
            i->native_width  = lua_tonumber(L, 3);
        if (lua_isnumber(L, 4))
            i->native_height = lua_tonumber(L, 4);
    }
    i->width  = i->native_width;
    i->height = i->native_height;
    if (memorize && !image_is_broken(i))
        task_image_pixbuf_from_file(i);

    luaL_getmetatable(L, IMAGE);
    lua_setmetatable(L, -2);
    return 1;
}

static inline void image_state_change(imageL* i, guint8 action)
{
    i->state = states[action][i->state];
}
static inline void image_scale_change(imageL* i, gint width, gint height)
{
    i->width  = MAX(1, width);
    i->height = MAX(1, height);
}
static inline void image_reset(imageL* i)
{
    i->state  = 0;
    i->width  = i->native_width;
    i->height = i->native_height;
}

static int rotate_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    gboolean clockwise = lua_toboolean(L, 2);
    guint8 action = (clockwise ? 1 : 3);

    image_state_change(i, action);
    image_emit_signal(i, L);

    return 0;
}
static int flip_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    gboolean horizontal = lua_toboolean(L, 2);
    guint action = (horizontal) ? 4 : 6;

    image_state_change(i, action);
    image_emit_signal(i, L);

    return 0;
}
static int set_state_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    guint8 state = luaL_checkinteger(L, 2);

    image_state_change(i, state);
    image_emit_signal(i, L);

    return 0;
}
static int scale_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    gint width  = luaL_checkinteger(L, 2);
    gint height = luaL_checkinteger(L, 3);

    image_scale_change(i, width, height);
    image_emit_signal(i, L);

    return 0;
}
static int reset_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);

    image_reset(i);
    image_emit_signal(i, L);

    return 0;
}

static int dump_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    const gchar* path = luaL_checkstring(L, 2);

    if (image_is_broken(i))
    {
        warn("Won't write «%s». Image is broken.", path);
        return 0;
    }

    GdkPixbufFormat* format = i->format;
    if (!gdk_pixbuf_format_is_writable(format))
    {
        info("«%s» is not a writable format. Saving «%s» as «png»", gdk_pixbuf_format_get_name(format), path);
        format = PNGformat;
    }
    const gchar* name = gdk_pixbuf_format_get_name(format);

    lua_pushvalue(L, 1);
    gint ref = luaL_ref(L, LUA_REGISTRYINDEX);
    task_dump_image(i, name, path, L, ref);
    return 0;
}

static int gc_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    g_free(i->path);
    g_object_unref(i->emitter);
    if (i->pxb)
        g_object_unref(i->pxb);
    return 0;
}
static int index_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_STR(  path,          i->path               );
    CASE_NUM(  width,         i->width              );
    CASE_NUM(  height,        i->height             );
    CASE_NUM(  native_width,  i->native_width       );
    CASE_NUM(  native_height, i->native_height      );
    CASE_NUM(  state,         i->state              );
    CASE_BOOL( swapped,       image_is_swapped(i)   );
    CASE_BOOL( broken,        image_is_broken(i)    );
    CASE_BOOL( writeable,     image_is_writable(i)  );
    CASE_BOOL( memorized,     image_is_memorized(i) );
    CASE_STR(  format,        image_format_name(i)  );

    CASE_FUNC( rotate,     image);
    CASE_FUNC( flip,       image);
    CASE_FUNC( set_state,  image);
    CASE_FUNC( scale,      image);
    CASE_FUNC( reset,      image);

    CASE_FUNC( dump,       image);

    return 1;
}

static const struct luaL_Reg imageLlib_f [] =
{
    {"new",  new_imageL },
    {"info", info_imageL},
    {NULL,   NULL       }
};
static const struct luaL_Reg imageLlib_m [] =
{
    {"__gc",    gc_imageL   },
    {"__index", index_imageL},
    {NULL,      NULL        }
};
int luaopen_imageL(lua_State *L)
{
    image_register_signal();
    luaL_newmetatable(L, IMAGE);
    luaL_setfuncs(L, imageLlib_m, 0);
    luaL_newlib(L, imageLlib_f, IMAGE);
    return 1;
}

