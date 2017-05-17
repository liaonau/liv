#include "luaL.h"
#include "util.h"
#include "imageL.h"
#include "resource.h"

static GdkPixbuf* BROKENpxb;

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
static inline void state_change(imageL* i, guint8 action)
{
    i->state = states[action][i->state];
}
static inline void scale_change(imageL* i, gint width, gint heigth)
{
    i->width  = MAX(1, width);
    i->height = MAX(1, height);
}

static inline gboolean image_is_swapped(imageL* i)
{
    return (image->state % 2);
}
static inline gboolean image_is_broken(imageL* i)
{
    return (i->pxb == BROKENpxb);
}

GdkPixbuf* image_create_pixbuf(imageL* i)
{
    GdkPixbuf* pxb;
    GdkPixbuf* scaledpxb;
    GdkPixbuf* tmppxb;
    gint w = gdk_pixbuf_get_width(i->pxb);
    gint h = gdk_pixbuf_get_height(i->pxb);
    if (w != i->width || h != i->height)
        scaledpxb = gdk_pixbuf_scale_simple(i->pxb, width, height, GDK_INTERP_BILINEAR);
    else
    {
        scaledpxb = i->pxb;
        g_object_ref(i->pxb);
    }
    switch (i->state)
    {
    case 0: //нейтральный элемент группы
        pxb = scaledpxb;
        g_object_ref(scaledpxb);
        break;
    case 1:
        pxb = gdk_pixbuf_rotate_simple(scaledpxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        break;
    case 2:
        pxb = gdk_pixbuf_rotate_simple(scaledpxb, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
        break;
    case 3:
        pxb = gdk_pixbuf_rotate_simple(scaledpxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        break;
    case 4:
        pxb = gdk_pixbuf_flip(scaledpxb, TRUE);
        break;
    case 5:
        tmppxb = gdk_pixbuf_rotate_simple(scaledpxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        pxb = gdk_pixbuf_flip(tmppxb, TRUE);
        g_object_unref(tmppxb);
        break;
    case 6:
        pxb = gdk_pixbuf_flip(scaledpxb, FALSE);
        break;
    case 7:
        tmppxb = gdk_pixbuf_rotate_simple(scaledpxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        pxb = gdk_pixbuf_flip(tmppxb, TRUE);
        g_object_unref(tmppxb);
        break;
    }
    g_object_unref(scaledpxb);
    return pxb;
}

static int new_imageL(lua_State *L)
{
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));
    const gchar* path = luaL_checkstring(L, 1);
    i->path = g_strdup(path);
    i->pxb  = gdk_pixbuf_new_from_file(i->path, NULL);
    if (!GDK_IS_PIXBUF(i->pxb))
    {
        g_object_ref(BROKENpxb);
        i->pxb = BROKENpxb;
    }
    i->width  = gdk_pixbuf_get_width(i->pxb);
    i->height = gdk_pixbuf_get_height(i->pxb);
    luaL_getmetatable(L, UDATA_IMAGEL);
    lua_setmetatable(L, -2);
    return 1;
}
static int rotate_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    GdkPixbufRotation rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
    gboolean clockwise = lua_toboolean(L, 2);
    if (!clockwise)
        rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;

    guint8 action = (clockwise ? 1 : 3);
    state_change(i, action);
    return 0;
}
static int flip_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    gboolean horizontal = lua_toboolean(L, 2);

    guint action = (horizontal) ? 4 : 6;
    state_change(i, action);
    return 0;
}
static int set_state_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    guint8 state = luaL_checkinteger(L, 2);

    state_change(i, state);
    return 0;
}
static int reset_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    state_change(i, 0);
    i->width  = gdk_pixbuf_get_width(i->pxb);
    i->height = gdk_pixbuf_get_height(i->pxb);
    return 0;
}
static int scale_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    gint width  = luaL_checkinteger(L, 2);
    gint height = luaL_checkinteger(L, 3);
    scale_change(i, width, height);
    return 0;
}

static int gc_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    g_object_unref(i->pxb);
    g_free(i->path);
    return 0;
}
static int tostring_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    lua_pushstring(L, i->path);
    return 1;
}
static int index_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_STR( L, field, "path",          i->path);
    CASE_NUM( L, field, "width",         i->width);
    CASE_NUM( L, field, "height",        i->height);
    CASE_NUM( L, field, "native_width",  gdk_pixbuf_get_width(i->pxb));
    CASE_NUM( L, field, "native_height", gdk_pixbuf_get_height(i->pxb));
    CASE_NUM( L, field, "state",         i->state);
    CASE_BOOL(L, field, "swapped",       image_is_swapped(i));
    CASE_BOOL(L, field, "broken",        image_is_broken(i));

    CASE_FUNC(L, field, "rotate",    UDATA_IMAGEL);
    CASE_FUNC(L, field, "flip",      UDATA_IMAGEL);
    CASE_FUNC(L, field, "set_state", UDATA_IMAGEL);
    CASE_FUNC(L, field, "scale",     UDATA_IMAGEL);
    CASE_FUNC(L, field, "reset",     UDATA_IMAGEL);

    return 1;
}

static const struct luaL_Reg imageLlib_f [] =
{
    {"new",  new_imageL },
    {NULL,  NULL        }
};
static const struct luaL_Reg imageLlib_m [] =
{
    {"__gc",       gc_imageL      },
    {"__tostring", tostring_imageL},
    {"__index",    index_imageL   },
    {NULL,         NULL           }
};
int luaopen_imageL(lua_State *L, const gchar* name)
{
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
    BROKENpxb = gdk_pixbuf_new_from_inline(-1, inline_broken, FALSE, NULL);
#pragma GCC diagnostic pop
    luaL_newmetatable(L, UDATA_IMAGEL);
    luaL_setfuncs(L, imageLlib_m, 0);
    luaL_newlib(L, imageLlib_f, name);
    return 1;
}

