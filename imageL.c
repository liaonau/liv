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


static GdkPixbuf* image_transform_pixbuf_by_state(imageL* i)
{
    GdkPixbuf* pxb = NULL;
    GdkPixbuf* secondpxb = NULL;

    switch (i->state)
    {
    case 0: //нейтральный элемент группы
        pxb = i->pxb;
        g_object_ref(pxb);
        break;
    case 1:
        pxb = gdk_pixbuf_rotate_simple(i->pxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        break;
    case 2:
        pxb = gdk_pixbuf_rotate_simple(i->pxb, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
        break;
    case 3:
        pxb = gdk_pixbuf_rotate_simple(i->pxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        break;
    case 4:
        pxb = gdk_pixbuf_flip(i->pxb, TRUE);
        break;
    case 5:
        secondpxb = gdk_pixbuf_rotate_simple(i->pxb, GDK_PIXBUF_ROTATE_CLOCKWISE);
        pxb = gdk_pixbuf_flip(secondpxb, TRUE);
        g_object_unref(secondpxb);
        break;
    case 6:
        pxb = gdk_pixbuf_flip(i->pxb, FALSE);
        break;
    case 7:
        secondpxb = gdk_pixbuf_rotate_simple(i->pxb, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        pxb = gdk_pixbuf_flip(secondpxb, TRUE);
        g_object_unref(secondpxb);
        break;
    }
    return pxb;
}

static void image_load_pixbuf(imageL* i)
{
    if (i->pxb && GDK_IS_PIXBUF(i->pxb))
        g_object_unref(i->pxb);

    i->pxb = gdk_pixbuf_new_from_file(i->path, NULL);
    if (!i->pxb || !GDK_IS_PIXBUF(i->pxb))
    {
        i->pxb    = brokenpbx;
        i->broken = TRUE;
    }
    else
        i->broken = FALSE;
    g_object_ref(i->pxb);

    state_reset(&i->state);
}


static int new_imageL(lua_State *L)
{
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));
    const gchar* path = luaL_checkstring(L, 1);
    i->path  = g_strdup(path);
    i->pxb   = NULL;
    i->image = (GtkImage*)gtk_image_new();
    g_object_ref(i->image);
    luaL_getmetatable(L, UDATA_IMAGEL);
    lua_setmetatable(L, -2);
    return 1;
}

static int load_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    image_load_pixbuf(i);
    gtk_image_set_from_pixbuf(i->image, i->pxb);
    return 0;
}

static int unload_imageL(lua_State* L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    if (i->pxb && GDK_IS_PIXBUF(i->pxb))
        g_object_unref(i->pxb);
    i->pxb    = NULL;
    i->broken = FALSE;
    gtk_image_clear(i->image);
    state_reset(&i->state);
    return 0;
}

static int rotate_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);

    GdkPixbufRotation rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
    gboolean clockwise = lua_toboolean(L, 2);
    if (!clockwise)
        rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;

    GdkPixbuf* pxb = gtk_image_get_pixbuf(i->image);
    if (GDK_IS_PIXBUF(pxb))
    {
        GdkPixbuf* newpxb = gdk_pixbuf_rotate_simple(pxb, rotation);
        gtk_image_set_from_pixbuf(i->image, newpxb);
        g_object_unref(newpxb);
        state_adjust_rotation(&i->state, clockwise);
    }
    return 0;
}

static int flip_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    gboolean horizontal = lua_toboolean(L, 2);

    GdkPixbuf* pxb = gtk_image_get_pixbuf(i->image);
    if (GDK_IS_PIXBUF(i->pxb))
    {
        GdkPixbuf* newpxb = gdk_pixbuf_flip(pxb, horizontal);
        gtk_image_set_from_pixbuf(i->image, newpxb);
        g_object_unref(newpxb);
        state_adjust_flipped(&i->state, horizontal);
    }
    return 0;
}

static void scale(imageL* i, gint width, gint height)
{
    if (width <= 0 || height <= 0)
        return;
    GdkPixbuf* pxb = gtk_image_get_pixbuf(i->image);
    if (!pxb || !GDK_IS_PIXBUF(pxb))
        return;
    gint w = gdk_pixbuf_get_width(pxb);
    gint h = gdk_pixbuf_get_height(pxb);
    if (w != width || h != height)
    {
        GdkPixbuf* adjpxb = image_transform_pixbuf_by_state(i);
        GdkPixbuf* newpxb = gdk_pixbuf_scale_simple(adjpxb, width, height, GDK_INTERP_BILINEAR);
        g_object_unref(adjpxb);
        gtk_image_set_from_pixbuf(i->image, newpxb);
        g_object_unref(newpxb);
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
    if (i->pxb && GDK_IS_PIXBUF(i->pxb))
    {
        gtk_image_set_from_pixbuf(i->image, i->pxb);
        state_reset(&i->state);
    }
    return 0;
}

static int gc_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    g_object_unref(i->pxb);
    gtk_widget_destroy((GtkWidget*)i->image);
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
        lua_pushnumber(L, gdk_pixbuf_get_width(gtk_image_get_pixbuf(i->image)));
    else if (g_strcmp0(field, "height") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_height(gtk_image_get_pixbuf(i->image)));
    else if (g_strcmp0(field, "native_width") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_width(i->pxb));
    else if (g_strcmp0(field, "native_height") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_height(i->pxb));
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
    else if (g_strcmp0(field, "load") == 0)
        lua_pushcfunction(L, load_imageL);
    else if (g_strcmp0(field, "unload") == 0)
        lua_pushcfunction(L, unload_imageL);
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

