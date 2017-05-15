#include "luaL.h"
#include "util.h"
#include "imageL.h"
#include "resource.h"

static GdkPixbuf* brokenpbx;

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

static void state_change(guint8* state, guint8 action)
{
    *state = states[action][*state];
}

static gboolean state_swapped(guint8* state)
{
    return (*state % 2);
}

static void state_reset(guint8* state)
{
    *state = 0;
}

static void state_adjust_rotation(guint8* state, gboolean clockwise)
{
    guint8 action = (clockwise ? 1 : 3);
    state_change(state, action);
}

static void state_adjust_flipped(guint8* state, gboolean horizontal)
{
    guint action = (horizontal) ? 4 : 6;
    state_change(state, action);
}


static void image_transform_pixbuf_by_state(imageL* i)
{
    GdkPixbuf* pxb = NULL;
    GdkPixbuf* secondpxb = NULL;

    if (i->state == 0)
        return;
    else
    {
        switch (i->state)
        {
        case 0: //нейтральный элемент группы
            break;
        case 1:
            pxb = gdk_pixbuf_rotate_simple(i->nativepxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
            break;
        case 2:
            pxb = gdk_pixbuf_rotate_simple(i->nativepxb, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
            break;
        case 3:
            pxb = gdk_pixbuf_rotate_simple(i->nativepxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
            break;
        case 4:
            pxb = gdk_pixbuf_flip(i->nativepxb, TRUE);
            break;
        case 5:
            secondpxb = gdk_pixbuf_rotate_simple(i->nativepxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
            g_object_unref(i->pxb);
            i->pxb = secondpxb;
            pxb = gdk_pixbuf_flip(i->pxb, TRUE);
            break;
        case 6:
            pxb = gdk_pixbuf_flip(i->nativepxb, FALSE);
            break;
        case 7:
            secondpxb = gdk_pixbuf_rotate_simple(i->nativepxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
            g_object_unref(i->pxb);
            i->pxb = secondpxb;
            pxb = gdk_pixbuf_flip(i->pxb, TRUE);
            break;
        }
        g_object_unref(i->pxb);
        i->pxb = pxb;
    }
}

static void image_load_pixbuf(imageL* i, const gchar* path)
{
    g_free(i->path);
    i->path = g_strdup(path);

    if (i->nativepxb && GDK_IS_PIXBUF(i->nativepxb))
        g_object_unref(i->nativepxb);
    if (i->pxb && GDK_IS_PIXBUF(i->pxb))
        g_object_unref(i->pxb);

    if (path)
        i->nativepxb = gdk_pixbuf_new_from_file(path, NULL);
    if (!i->nativepxb || !GDK_IS_PIXBUF(i->nativepxb))
    {
        i->nativepxb = brokenpbx;
        i->broken = TRUE;
    }
    else
        i->broken = FALSE;
    g_object_ref(i->nativepxb);
    i->pxb = i->nativepxb;
    g_object_ref(i->pxb);

    state_reset(&i->state);
}


static int new_imageL(lua_State *L)
{
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));
    const gchar* path = luaL_checkstring(L, 1);
    i->nativepxb = NULL;
    i->pxb       = NULL;
    i->path      = NULL;
    image_load_pixbuf(i, path);
    luaL_getmetatable(L, UDATA_IMAGEL);
    lua_setmetatable(L, -2);
    return 1;
}

static int reload_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    const gchar* path = luaL_checkstring(L, 2);
    image_load_pixbuf(i, path);
    return 0;
}

static int rotate_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);

    GdkPixbufRotation rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
    gboolean clockwise = lua_toboolean(L, 2);
    if (!clockwise)
        rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;

    if (GDK_IS_PIXBUF(i->pxb))
    {
        GdkPixbuf* pxb = gdk_pixbuf_rotate_simple(i->pxb, rotation);
        g_object_unref(i->pxb);
        i->pxb = pxb;
        state_adjust_rotation(&i->state, clockwise);
    }
    return 0;
}

static int flip_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);

    gboolean horizontal = lua_toboolean(L, 2);
    if (GDK_IS_PIXBUF(i->pxb))
    {
        GdkPixbuf* pxb = gdk_pixbuf_flip(i->pxb, horizontal);
        g_object_unref(i->pxb);
        i->pxb = pxb;
        state_adjust_flipped(&i->state, horizontal);
    }
    return 0;
}

static void scale(imageL* i, gint width, gint height)
{
    if (width <= 0 || height <= 0)
        return;
    gint w = gdk_pixbuf_get_width(i->pxb);
    gint h = gdk_pixbuf_get_height(i->pxb);
    if (w != width || h != height)
    {
        image_transform_pixbuf_by_state(i);
        GdkPixbuf* pxb;
        pxb = gdk_pixbuf_scale_simple(i->pxb, width, height, GDK_INTERP_BILINEAR);
        g_object_unref(i->pxb);
        i->pxb = pxb;
    }
}

static int scale_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    gint width  = luaL_checkinteger(L, 2);
    gint height = luaL_checkinteger(L, 3);
    scale(i, width, height);
    return 0;
}

static int reset_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    g_object_unref(i->pxb);
    i->pxb = i->nativepxb;
    g_object_ref(i->nativepxb);
    state_reset(&i->state);
    return 0;
}

static int gc_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    g_object_unref(i->pxb);
    g_object_unref(i->nativepxb);
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
    if (     g_strcmp0(field, "path") == 0)
        lua_pushstring(L, i->path);
    else if (g_strcmp0(field, "width") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_width(i->pxb));
    else if (g_strcmp0(field, "height") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_height(i->pxb));
    else if (g_strcmp0(field, "native_width") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_width(i->nativepxb));
    else if (g_strcmp0(field, "native_height") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_height(i->nativepxb));
    else if (g_strcmp0(field, "state") == 0)
        lua_pushnumber(L, i->state);
    else if (g_strcmp0(field, "swapped") == 0)
        lua_pushboolean(L, state_swapped(&i->state));
    else if (g_strcmp0(field, "broken") == 0)
        lua_pushboolean(L, i->broken);
    else if (g_strcmp0(field, "rotate") == 0)
        lua_pushcfunction(L, rotate_imageL);
    else if (g_strcmp0(field, "flip") == 0)
        lua_pushcfunction(L, flip_imageL);
    else if (g_strcmp0(field, "scale") == 0)
        lua_pushcfunction(L, scale_imageL);
    else if (g_strcmp0(field, "reset") == 0)
        lua_pushcfunction(L, reset_imageL);
    else if (g_strcmp0(field, "reload") == 0)
        lua_pushcfunction(L, reload_imageL);
    else
        lua_pushnil(L);
    return 1;
}

static int newindex_imageL(lua_State *L)
{
    /*imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);*/
    /*const gchar* field = luaL_checkstring(L, 2);*/
    /*if (g_strcmp0(field, "id") == 0)*/
    /*{*/
        /*gint idx = luaL_checknumber(L, 3);*/
        /*i->id = idx;*/
    /*}*/
    return 0;
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
    {"__newindex", newindex_imageL},
    {NULL,         NULL           }
};

int luaopen_imageL(lua_State *L, const gchar* name)
{
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
    brokenpbx = gdk_pixbuf_new_from_inline(-1, inline_broken, FALSE, NULL);
#pragma GCC diagnostic pop
    luaL_newmetatable(L, UDATA_IMAGEL);
    luaL_setfuncs(L, imageLlib_m, 0);
    luaL_newlib(L, imageLlib_f, name);
    return 1;
}

