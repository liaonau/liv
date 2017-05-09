#include "luaL.h"
#include "fileL.h"
#include "imageL.h"
#include "util.h"

static const gchar* xpm_broken[] =
{
/* columns rows colors chars-per-pixel */
"32 32 4 1 ",
"  c #010101",
". c #020201",
"X c #020202",
"o c None",
/* pixels */
"oooooooooooooooooooooooooooooooo",
"oooooooooooooooooooooooooooooooo",
"ooooo                      ooooo",
"ooooo  ooooooooooooooooooX ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo ooooooXXoooooooooooo ooooo",
"ooooo oooooX   ooooooooooo ooooo",
"ooooo ooooX XX  oooooooooo ooooo",
"ooooo XooX XooX  oooooXXX  ooooo",
"ooooo     XooooX  ooo      ooooo",
"ooooo  XXXooooooX  o  oooooooooo",
"ooooooooooooX oooX   ooooooooooo",
"ooooooooooo    oooX oooooooooooo",
"oooooooooo  Xo  oooooooooooooooo",
"ooooooooo  Xooo  ooooo     ooooo",
"ooooo     Xooooo  ooo   X  ooooo",
"ooooo Xoooooooooo  X  oooo ooooo",
"ooooo oooooooooooo   ooooo ooooo",
"ooooo ooooooooooooo oooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo oooooooooooooooooooo ooooo",
"ooooo XooooooooooooooooooX ooooo",
"ooooo                      ooooo",
"oooooooooooooooooooooooooooooooo",
"oooooooooooooooooooooooooooooooo"
};

static int new_fileL(lua_State *L)
{
    fileL* f;
    const gchar* path = luaL_checkstring(L, -1);
    f = (fileL*)lua_newuserdata(L, sizeof(fileL));

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (!GDK_IS_PIXBUF(pixbuf))
    {
        warn("can't load %s", path);
        /*pixbuf = gdk_pixbuf_new_from_file(BROKEN, NULL);*/
        pixbuf = gdk_pixbuf_new_from_xpm_data(xpm_broken);
    }

    f->path   = g_strdup(path);
    f->pixbuf = pixbuf;
    g_object_ref(pixbuf);

    luaL_getmetatable(L, UDATA_FILEL);
    lua_setmetatable(L, -2);
    return 1;
}

static int get_fileL(lua_State *L)
{
    fileL *f = (fileL*)luaL_checkudata(L, 1, UDATA_FILEL);
    const gchar* field = luaL_checkstring(L, 2);
    if (g_strcmp0(field, "path") == 0)
        lua_pushstring(L, f->path);
    else if (g_strcmp0(field, "width") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_width(f->pixbuf));
    else if (g_strcmp0(field, "height") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_height(f->pixbuf));
    else if (g_strcmp0(field, "aspect") == 0)
        lua_pushnumber(L, gdk_pixbuf_get_width(f->pixbuf)/(double)gdk_pixbuf_get_height(f->pixbuf));
    else
        lua_pushnil(L);
    return 1;
}

static int tostring_fileL(lua_State *L)
{
    fileL *f = (fileL*)luaL_checkudata(L, 1, UDATA_FILEL);
    lua_pushstring(L, f->path);
    return 1;
}

static int gc_fileL(lua_State *L)
{
    fileL *f = (fileL*)luaL_checkudata(L, 1, UDATA_FILEL);
    g_object_unref(f->pixbuf);
    return 0;
}

static const struct luaL_Reg fileLlib_f [] =
{
    {"new", new_fileL},
    {NULL,  NULL}
};

static const struct luaL_Reg fileLlib_m [] =
{
    {"__index",    get_fileL     },
    {"__tostring", tostring_fileL},
    {"__gc",       gc_fileL      },
    {NULL,  NULL}
};

int luaopen_fileL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, UDATA_FILEL);
    luaL_setfuncs(L, fileLlib_m, 0);
    luaL_newlib(L, fileLlib_f, name);
    return 1;
}

