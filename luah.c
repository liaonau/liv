#include "conf.h"
#include "luah.h"

#define DEF_UINT 64

gboolean luaH_init(void)
{
    L = luaL_newstate();
    if (L == NULL)
        return FALSE;
    luaL_openlibs(L);
    return TRUE;
}

gboolean luaH_loadrc(void)
{
    const char* confpath = "./rc.lua";
    if (luaL_dofile(L, confpath) != 0)
    {
        fprintf(stderr, "couldn't parse config file \"%s\"\n", confpath);
        return FALSE;
    }
    return TRUE;
}

guint luaH_get_uint_opt(const gchar* opt)
{
    guint ret = DEF_UINT;
    lua_getglobal(L, "opts");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, opt);
        if (lua_isnumber(L, -1))
        {
            ret = (guint)lua_tonumber(L, -1);
            lua_pop(L, -1);
        }
    }
    lua_pop(L, -1);
    return ret;
}
