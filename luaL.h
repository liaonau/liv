#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define CASE_FUNC(name, udatatype)                   \
    if (g_strcmp0(field, #name) == 0)                \
    {                                                \
        lua_pushcfunction(L, name##_##udatatype##L); \
        return 1;                                    \
    }                                                \

#define CASE_NUM(name, value)                        \
    if (g_strcmp0(field, #name) == 0)                \
    {                                                \
        lua_pushnumber(L, (value));                  \
        return 1;                                    \
    }                                                \

#define CASE_BOOL(name, value)                       \
    if (g_strcmp0(field, #name) == 0)                \
    {                                                \
        lua_pushboolean(L, (value));                 \
        return 1;                                    \
    }                                                \

#define CASE_STR(name, value)                        \
    if (g_strcmp0(field, #name) == 0)                \
    {                                                \
        lua_pushstring(L, (value));                  \
        return 1;                                    \
    }                                                \

#define INDEX_FIELD(name, udatatype)    \
    if (g_strcmp0(field, #name) == 0)   \
    {                                   \
        lua_remove(L, 2);               \
        name##_get_##udatatype##L(L);   \
        return 1;                       \
    }

#define NEWINDEX_FIELD(name, udatatype) \
    if (g_strcmp0(field, #name) == 0)   \
    {                                   \
        lua_remove(L, 2);               \
        name##_set_##udatatype##L(L);   \
        return 0;                       \
    }

void luaL_setfuncs (lua_State*, const luaL_Reg*, int);
void luaL_newlib(lua_State*, const luaL_Reg*, const char*);
