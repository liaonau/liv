#include "luaL.h"
#include "util.h"
#include "imageL.h"
#include "resource.h"
#include "idle.h"

/*static GdkPixbuf*       BROKENpxb;*/
static GdkPixbufFormat* PNGformat;

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
static inline void scale_change(imageL* i, gint width, gint height)
{
    i->width  = MAX(1, width);
    i->height = MAX(1, height);
}

static inline gboolean image_is_swapped(imageL* i)
{
    return (i->state % 2);
}
static inline gboolean image_is_broken(imageL* i)
{
    return (i->broken);
}
static inline gboolean image_is_memorized(imageL* i)
{
    return (i->memorize);
}
static inline gboolean image_is_writable(imageL* i)
{
    return (!image_is_broken(i) && gdk_pixbuf_format_is_writable(i->format));
}
static inline const gchar* image_format_name(imageL* i)
{
    return (image_is_broken(i) ? NULL : gdk_pixbuf_format_get_name(i->format));
}

static void image_set_broken(imageL* i)
{
    if (!i->broken)
        g_object_ref(BROKENpxb);
    i->broken        = TRUE;
    if (i->memorize)
    i->pxb           = BROKENpxb;
    i->native_width  = gdk_pixbuf_get_width(BROKENpxb);
    i->native_height = gdk_pixbuf_get_height(BROKENpxb);
    i->format        = NULL;
    i->state         = 0;
    i->width         = i->native_width;
    i->height        = i->native_height;
}
static void image_set_not_broken(imageL* i)
{
    if (i->broken)
    {
        g_object_unref(BROKENpxb);
        i->state  = 0;
        i->width  = i->native_width;
        i->height = i->native_height;
    }
    if (!i->memorize)
        i->pxb = NULL;
    i->broken = FALSE;
}
static void image_update_info(imageL* i)
{
    i->format = gdk_pixbuf_get_file_info(i->path, &i->native_width, &i->native_height);
    if (G_UNLIKELY(!i->format))
        image_set_broken(i);
    else
        image_set_not_broken(i);
}
static GdkPixbuf* image_pixbuf_from_file(imageL* i)
{
    return gdk_pixbuf_new_from_file(i->path, NULL);
}
#define IMAGE_GET_FILED_PIXBUF(i, pxb) \
{\
    pxb = gdk_pixbuf_new_from_file(((i)->path), NULL); \
    if (G_UNLIKELY(!GDK_IS_PIXBUF((pxb)))) \
    { \
        image_set_broken((i));\
        g_object_ref(BROKENpxb);\
        return BROKENpxb;\
    } \
}
static inline GdkPixbuf* image_get_scaled_pixbuf(imageL *i, GdkPixbuf* startpxb)
{
    GdkPixbuf* pxb;
    gint w = gdk_pixbuf_get_width(startpxb);
    gint h = gdk_pixbuf_get_height(startpxb);
    if (w == i->width && h == i->height)
    {
        pxb = startpxb;
        g_object_ref(pxb);
    }
    else
        pxb = gdk_pixbuf_scale_simple(startpxb, i->width, i->height, GDK_INTERP_BILINEAR);
    g_object_unref(startpxb);
    return pxb;
}
static inline GdkPixbuf* image_get_stated_pixbuf(imageL *i, GdkPixbuf* startpxb)
{
    GdkPixbuf* pxb;
    GdkPixbuf* tmppxb;
    switch (i->state)
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

GdkPixbuf* image_get_pixbuf(imageL* i)
{
    GdkPixbuf* pxb;
    GdkPixbuf* nativepxb;
    GdkPixbuf* tmppxb;

    if (i->memorize)
    {
        if (G_LIKELY(i->pxb))
        {
            nativepxb = i->pxb;
            g_object_ref(nativepxb);
            if (i->broken)
                return nativepxb;
        }
        else //deferred, потенциально не единожды прочитанный файл ☹
        {
            info("плохо, лишняя работа с «%s»", i->path);
            IMAGE_GET_FILED_PIXBUF(i, nativepxb);
        }
    }
    else
    {
        image_update_info(i);
        if (G_UNLIKELY(i->broken))
        {
            g_object_ref(BROKENpxb);
            return BROKENpxb;
        }
        else
            IMAGE_GET_FILED_PIXBUF(i, nativepxb);
    }

    tmppxb = image_get_scaled_pixbuf(i, nativepxb);
    pxb    = image_get_stated_pixbuf(i, tmppxb);
    return pxb;
}

static int new_imageL(lua_State *L)
{
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));
    const gchar* path = luaL_checkstring(L, 1);
    gboolean memorize = lua_toboolean(L, 2);
    i->memorize = memorize;
    i->path     = g_strdup(path);
    i->pxb      = NULL;
    i->broken   = FALSE;
    image_update_info(i);
    i->width  = i->native_width;
    i->height = i->native_height;
    i->state  = 0;
    if (i->memorize && !image_is_broken(i))
        idle_pixbuf(i, &image_pixbuf_from_file, i, &image_set_broken);
    luaL_getmetatable(L, IMAGE);
    lua_setmetatable(L, -2);
    return 1;
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

static int rotate_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    gboolean clockwise = lua_toboolean(L, 2);

    guint8 action = (clockwise ? 1 : 3);
    state_change(i, action);
    return 0;
}
static int flip_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    gboolean horizontal = lua_toboolean(L, 2);

    guint action = (horizontal) ? 4 : 6;
    state_change(i, action);
    return 0;
}
static int set_state_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    guint8 state = luaL_checkinteger(L, 2);

    state_change(i, state);
    return 0;
}
static int reset_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    i->state  = 0;
    i->width  = i->native_width;
    i->height = i->native_height;
    return 0;
}
static int scale_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    gint width  = luaL_checkinteger(L, 2);
    gint height = luaL_checkinteger(L, 3);
    scale_change(i, width, height);
    return 0;
}

static int fork_imageL(lua_State *L)
{
    imageL* p = (imageL*)luaL_checkudata(L, 1, IMAGE);
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));
    i->path          = g_strdup(p->path);
    idle_pixbuf(i, &image_get_pixbuf, p, NULL);
    i->memorize      = p->memorize;
    i->broken        = p->broken;
    i->format        = p->format;
    i->native_width  = p->width;
    i->native_height = p->height;
    i->width         = p->width;
    i->height        = p->height;
    i->state         = 0;
    luaL_getmetatable(L, IMAGE);
    lua_setmetatable(L, -2);
    return 1;
}
/*static imageL* image_clone(lua_State* L, imageL* i)*/
/*{*/
    /*imageL* c = (imageL*)lua_newuserdata(L, sizeof(imageL));*/
    /*c->path          = g_strdup(i->path);*/
    /*c->pxb           = i->pxb;*/
    /*c->memorize      = i->memorize;*/
    /*c->broken        = i->broken;*/
    /*c->native_width  = i->native_width;*/
    /*c->native_height = i->native_height;*/
    /*c->width         = i->width;*/
    /*c->height        = i->height;*/
    /*c->state         = i->state;*/
    /*luaL_getmetatable(L, IMAGE);*/
    /*lua_setmetatable(L, -2);*/
/*}*/
static int dump_imageL(lua_State* L)
{
    imageL* i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    const gchar* path = luaL_checkstring(L, 2);

    GdkPixbuf* pxb = image_get_pixbuf(i);
    if (image_is_broken(i))
    {
        warn("Won't write «%s». Image is broken.", path);
        g_object_unref(pxb);
        return 0;
    }

    gboolean success = FALSE;
    GdkPixbufFormat* format = i->format;
    if (!gdk_pixbuf_format_is_writable(format))
    {
        info("«%s» is not a writable format. Saving «%s» as «png»", gdk_pixbuf_format_get_name(format), path);
        format = PNGformat;
    }
    const gchar* name = gdk_pixbuf_format_get_name(format);
    /*GdkPixbuf* pxb = image_get_pixbuf(i);*/
    GError* error = NULL;
    gdk_pixbuf_save(pxb, path, name, &error, NULL);
    if (error != NULL)
    {
        warn("cannot save «%s». %s", path, error->message);
        g_error_free(error);
    }
    else
    {
        info("saved «%s».", path);
        success = TRUE;
    }
    g_object_unref(pxb);
    lua_pushboolean(L, success);
    return 1;
}

static int gc_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    g_free(i->path);
    if (i->pxb)
    {
        g_object_unref(i->pxb);
        i->pxb = NULL;
    }
    return 0;
}
static int index_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, IMAGE);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_NUM( L, field, width,         i->width);
    CASE_NUM( L, field, height,        i->height);
    CASE_NUM( L, field, native_width,  i->native_width);
    CASE_NUM( L, field, native_height, i->native_height);
    CASE_NUM( L, field, state,         i->state);
    CASE_BOOL(L, field, swapped,       image_is_swapped(i));
    CASE_BOOL(L, field, broken,        image_is_broken(i));
    CASE_BOOL(L, field, writeable,     image_is_writable(i));
    CASE_BOOL(L, field, memorize,      image_is_memorized(i));
    CASE_STR( L, field, format,        image_format_name(i));

    CASE_FUNC(L, field, rotate,    image);
    CASE_FUNC(L, field, flip,      image);
    CASE_FUNC(L, field, set_state, image);
    CASE_FUNC(L, field, scale,     image);
    CASE_FUNC(L, field, reset,     image);

    CASE_FUNC(L, field, fork,      image);
    CASE_FUNC(L, field, dump,      image);

    return 1;
}

static const struct luaL_Reg imageLlib_f [] =
{
    {"new",   new_imageL },
    {"info",  info_imageL},
    {NULL,    NULL       }
};
static const struct luaL_Reg imageLlib_m [] =
{
    {"__gc",       gc_imageL      },
    {"__index",    index_imageL   },
    {NULL,         NULL           }
};
int luaopen_imageL(lua_State *L, const gchar* name)
{
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #pragma GCC diagnostic push
    BROKENpxb = gdk_pixbuf_new_from_inline(-1, inline_broken, FALSE, NULL);
    #pragma GCC diagnostic pop
    GSList* formats = gdk_pixbuf_get_formats();
    GSList* f = formats;
    while(f)
    {
        GdkPixbufFormat* format = (GdkPixbufFormat*)f->data;
        if (g_strcmp0(gdk_pixbuf_format_get_name(format), "png") == 0)
        {
            PNGformat = format;
            break;
        }
        f = f->next;
    }
    g_slist_free(formats);

    luaL_newmetatable(L, IMAGE);
    luaL_setfuncs(L, imageLlib_m, 0);
    luaL_newlib(L, imageLlib_f, name);
    return 1;
}

