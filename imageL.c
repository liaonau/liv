#include "luaL.h"
#include "util.h"
#include "imageL.h"
#include "graph.h"
#include "resource.h"

static const GdkPixbuf* comppbx;
static const GdkPixbuf* brokenpbx;

//таблица умножения группы четырех вращений и вертикального и горизонтального отражения
//a0-a3 — вращения, a4,a6 — горизонтальные и вертикальные отражения, a5 — транспонирование
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

static void change_state(guint8* state, guint8 action)
{
    *state = states[action][*state];
}

static gboolean swapped_state(guint8* state)
{
    return (*state % 2);
}

static void reset_state(guint8* state)
{
    *state = 0;
}

static void adjust_state_rotation(guint8* state, gboolean clockwise)
{
    guint8 action = (clockwise ? 1 : 3);
    change_state(state, action);
}

static void adjust_state_flipped(guint8* state, gboolean horizontal)
{
    guint action = (horizontal) ? 4 : 6;
    change_state(state, action);
}

static GdkPixbuf* new_pixbuf_by_state(guint8* state, GdkPixbuf* original)
{
    GdkPixbuf* tmppxb = gdk_pixbuf_copy(original);
    gdk_pixbuf_copy_options(original, tmppxb);
    switch (*state)
    {
    case 0: //нейтральный элемент группы
        break;
    case 1:
        tmppxb = gdk_pixbuf_rotate_simple(tmppxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        break;
    case 2:
        tmppxb = gdk_pixbuf_rotate_simple(tmppxb, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
        break;
    case 3:
        tmppxb = gdk_pixbuf_rotate_simple(tmppxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        break;
    case 4:
        tmppxb = gdk_pixbuf_flip(tmppxb, TRUE);
        break;
    case 5: //транспонирование a1*a6=a5
        tmppxb = gdk_pixbuf_rotate_simple(tmppxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        tmppxb = gdk_pixbuf_flip(tmppxb, TRUE);
        break;
    case 6:
        tmppxb = gdk_pixbuf_flip(tmppxb, FALSE);
        break;
    case 7: //a3*a6=a7
        tmppxb = gdk_pixbuf_rotate_simple(tmppxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        tmppxb = gdk_pixbuf_flip(tmppxb, TRUE);
        break;
    }
    return tmppxb;
}


static int new_imageL(lua_State *L)
{
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));
    const gchar* path = lua_tostring(L, 1);
    i->path = g_strdup(path);

    GdkPixbuf* pixbuf;
    if (path)
        pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (!GDK_IS_PIXBUF(pixbuf))
    {
        pixbuf = gdk_pixbuf_copy(brokenpbx);
        i->broken = TRUE;
    }
    else
        i->broken = FALSE;
    i->originalpxb = pixbuf;
    g_object_ref(pixbuf);
    i->image  = (GtkImage*)gtk_image_new_from_pixbuf(pixbuf);
    g_object_ref(i->image);
    i->marked = FALSE;

    reset_state(&i->state);

    luaL_getmetatable(L, UDATA_IMAGEL);
    lua_setmetatable(L, -2);
    return 1;
}

static int toggle_mark_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    GdkPixbuf* buf  = gtk_image_get_pixbuf(i->image);
    int dest_width  = gdk_pixbuf_get_width(buf);
    int dest_height = gdk_pixbuf_get_height(buf);
    i->realpxb = gdk_pixbuf_copy(buf);
    gdk_pixbuf_copy_options(i->realpxb, buf);
    g_object_ref(i->realpxb);
    gdk_pixbuf_composite(
            gdk_pixbuf_scale_simple(comppbx, dest_width, dest_height, GDK_INTERP_BILINEAR),
            buf,
            0, //int dest_x,
            0, //int dest_y,
            dest_width,
            dest_height,
            0, //double offset_x,
            0, //double offset_y,
            1, //double scale_x,
            1, //double scale_y,
            GDK_INTERP_BILINEAR, //GdkInterpType interp_type,
            255 //int overall_alpha
            );
    gtk_image_set_from_pixbuf(i->image, buf);
    return 0;
}

static int rotate_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(i->image);

    GdkPixbufRotation rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
    gboolean clockwise = lua_toboolean(L, 2);
    if (!clockwise)
        rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;

    if (GDK_IS_PIXBUF(pixbuf))
    {
        gtk_image_set_from_pixbuf(i->image, gdk_pixbuf_rotate_simple(pixbuf, rotation));
        adjust_state_rotation(&i->state, clockwise);
    }
    return 0;
}

static int flip_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);

    gboolean horizontal = lua_toboolean(L, 2);
    GdkPixbuf* old_pixbuf = gtk_image_get_pixbuf(i->image);
    if (GDK_IS_PIXBUF(old_pixbuf))
    {
        GdkPixbuf* new_pixbuf = gdk_pixbuf_flip(old_pixbuf, horizontal);
        if (GDK_IS_PIXBUF(new_pixbuf))
        {
            gtk_image_set_from_pixbuf(i->image, new_pixbuf);
            adjust_state_flipped(&i->state, horizontal);
        }
    }
    return 0;
}

static void scale(imageL* i, gint width, gint height)
{
    if (width <= 0 || height <= 0)
        return;
    GdkPixbuf* currentpbx = gtk_image_get_pixbuf(i->image);
    gint w = gdk_pixbuf_get_width(currentpbx);
    gint h = gdk_pixbuf_get_height(currentpbx);
    if (w != width || h != height)
    {
        GdkPixbuf* pixbuf = new_pixbuf_by_state(&i->state, i->originalpxb);
        pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf(i->image, pixbuf);
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
    gtk_image_set_from_pixbuf(i->image, i->originalpxb);
    reset_state(&i->state);
    return 0;
}

static int gc_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    g_object_unref(i->image);
    g_object_unref(i->originalpxb);
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
        lua_pushnumber(L, gdk_pixbuf_get_width(gtk_image_get_pixbuf(i->image)));
    else if (g_strcmp0(field, "height") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_height(gtk_image_get_pixbuf(i->image)));
    else if (g_strcmp0(field, "native_width") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_width(i->originalpxb));
    else if (g_strcmp0(field, "native_height") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_height(i->originalpxb));
    else if (g_strcmp0(field, "state") == 0)
        lua_pushnumber(L, i->state);
    else if (g_strcmp0(field, "swapped") == 0)
        lua_pushboolean(L, swapped_state(&i->state));
    else if (g_strcmp0(field, "broken") == 0)
        lua_pushboolean(L, i->broken);
    else if (g_strcmp0(field, "marked") == 0)
        lua_pushboolean(L, i->marked);
    else if (g_strcmp0(field, "rotate") == 0)
        lua_pushcfunction(L, rotate_imageL);
    else if (g_strcmp0(field, "flip") == 0)
        lua_pushcfunction(L, flip_imageL);
    else if (g_strcmp0(field, "scale") == 0)
        lua_pushcfunction(L, scale_imageL);
    else if (g_strcmp0(field, "reset") == 0)
        lua_pushcfunction(L, reset_imageL);
    else if (g_strcmp0(field, "mark") == 0)
        lua_pushcfunction(L, mark_imageL);
    else if (g_strcmp0(field, "unmark") == 0)
        lua_pushcfunction(L, unmark_imageL);
    else
        lua_pushnil(L);
    return 1;
}

static int newindex_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    const gchar* field = luaL_checkstring(L, 2);
    if (     g_strcmp0(field, "width") == 0)
    {
        gint w = luaL_checknumber(L, 3);
        scale(i, w, gdk_pixbuf_get_height(gtk_image_get_pixbuf(i->image)));
    }
    else if (g_strcmp0(field, "height") == 0)
    {
        gint h = luaL_checknumber(L, 3);
        scale(i, gdk_pixbuf_get_width(gtk_image_get_pixbuf(i->image)), h);
    }
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
    comppbx   = gdk_pixbuf_new_from_inline(-1, inline_frame,  FALSE, NULL);
#pragma GCC diagnostic pop
    luaL_newmetatable(L, UDATA_IMAGEL);
    luaL_setfuncs(L, imageLlib_m, 0);
    luaL_newlib(L, imageLlib_f, name);
    return 1;
}

