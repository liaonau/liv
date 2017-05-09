#include "luaL.h"
#include "util.h"
#include "imageL.h"
#include "graph.h"

static int new_imageL(lua_State *L)
{
    fileL*  f = luaL_checkudata(L, 1, UDATA_FILEL);
    imageL* i = (imageL*)lua_newuserdata(L, sizeof(imageL));

    GdkPixbuf* pixbuf = f->pixbuf;
    i->image  = (GtkImage*)gtk_image_new_from_pixbuf(pixbuf);
    i->pixbuf = pixbuf;
    g_object_ref(i->image);
    g_object_ref(i->pixbuf);

    luaL_getmetatable(L, UDATA_IMAGEL);
    lua_setmetatable(L, -2);
    return 1;
}

static int copy_imageL(lua_State *L)
{
    imageL *src = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    imageL* dst = (imageL*)lua_newuserdata(L, sizeof(imageL));

    dst->pixbuf = src->pixbuf;
    g_object_ref(dst->pixbuf);

    dst->image = (GtkImage*)gtk_image_new_from_pixbuf(gtk_image_get_pixbuf(src->image));
    g_object_ref(dst->image);

    luaL_getmetatable(L, UDATA_IMAGEL);
    lua_setmetatable(L, -2);
    return 1;
}

static int rotate_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    GtkImage*  image  = i->image;
    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(image);

    GdkPixbufRotation rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
    gboolean clockwise = lua_toboolean(L, 2);
    if (!clockwise)
        rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;

    if (GDK_IS_PIXBUF(pixbuf))
        gtk_image_set_from_pixbuf(image, gdk_pixbuf_rotate_simple(pixbuf, rotation));
    return 0;
}

static int flip_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    GtkImage* image = i->image;

    gboolean horizontal = lua_toboolean(L, 2);
    GdkPixbuf* old_pixbuf = gtk_image_get_pixbuf(image);
    if (GDK_IS_PIXBUF(old_pixbuf))
    {
        GdkPixbuf* new_pixbuf = gdk_pixbuf_flip(old_pixbuf, horizontal);
        if (GDK_IS_PIXBUF(new_pixbuf))
            gtk_image_set_from_pixbuf(image, new_pixbuf);
    }
    return 0;
}

static int scale_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    gint width  = luaL_checkinteger(L, 2);
    gint height = luaL_checkinteger(L, 3);
    if (width <= 0 || height <= 0)
        return 0;
    GtkImage* image = i->image;
    GdkPixbuf* old_pixbuf = gtk_image_get_pixbuf(image);
    if (GDK_IS_PIXBUF(old_pixbuf))
    {
        gint old_width  = gdk_pixbuf_get_width(old_pixbuf);
        gint old_height = gdk_pixbuf_get_height(old_pixbuf);
        if (old_width != width || old_height != height)
        {
            GdkPixbuf* new_pixbuf = gdk_pixbuf_scale_simple(old_pixbuf, width, height, GDK_INTERP_BILINEAR);
            if (GDK_IS_PIXBUF(new_pixbuf))
                gtk_image_set_from_pixbuf(image, new_pixbuf);
        }
    }
    return 0;
}

static int reset_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    gtk_image_set_from_pixbuf(i->image, i->pixbuf);
    return 0;
}

static int gc_imageL(lua_State *L)
{
    imageL *i = (imageL*)luaL_checkudata(L, 1, UDATA_IMAGEL);
    g_object_unref(i->image);
    g_object_unref(i->pixbuf);
    warn("image: collecting garbage");
    return 0;
}

static const struct luaL_Reg imageLlib_f [] =
{
    {"new",  new_imageL },
    {"copy", copy_imageL},
    {NULL,  NULL        }
};

static const struct luaL_Reg imageLlib_m [] =
{
    {"rotate", rotate_imageL},
    {"flip",   flip_imageL  },
    {"scale",  scale_imageL },
    {"reset",  reset_imageL },
    {NULL,     NULL         }
};

static const struct luaL_Reg imageLlib_gc [] =
{
    {"__gc",  gc_imageL},
    {NULL,     NULL    }
};

int luaopen_imageL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, UDATA_IMAGEL);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, imageLlib_m, 0);
    luaL_setfuncs(L, imageLlib_gc, 0);
    luaL_newlib(L, imageLlib_f, name);
    return 1;
}

