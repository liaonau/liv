#include "luaL.h"
#include "util.h"

static int title_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    gchar* title = g_strconcat(APPNAME, " ", luaL_optstring(L, 2, ""), NULL);
    gtk_window_set_title(window, title);
    g_free(title);
    return 0;
}

static int geometry_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    gint w, h;
    gtk_window_get_size(window, &w, &h);
    lua_newtable(L);
    lua_pushnumber(L, w);
    lua_setfield(L, -2, "width");
    lua_pushnumber(L, h);
    lua_setfield(L, -2, "height");
    return 1;
}

static int tostring_appL(lua_State *L)
{
    luaL_checkudata(L, 1, UDATA_APPL);
    lua_pushstring(L, APPNAME);
    return 1;
}

static const struct luaL_Reg appLlib_m [] =
{
    {"geometry", geometry_appL},
    {"title",    title_appL   },
    {NULL,       NULL         }
};

static const struct luaL_Reg appLlib_ts [] =
{
    {"__tostring", tostring_appL},
    {NULL,         NULL         }
};

int luaopen_appL(lua_State *L, const gchar* name)
{
    lua_newuserdata(L, sizeof(appL));
    luaL_newmetatable(L, UDATA_APPL);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -3);
    luaL_setfuncs(L, appLlib_m, 0);
    luaL_setfuncs(L, appLlib_ts, 0);
    lua_pop(L, 1);
    lua_setglobal(L, name);
    return 1;
}

