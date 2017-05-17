#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define luaL_getreg(L, p) \
{ \
    lua_pushlightuserdata(L, (void*)(p)); \
    lua_gettable(L, LUA_REGISTRYINDEX); \
};
#define luaL_setreg(L, idx, p) \
{ \
    lua_pushlightuserdata(L, (void*)(p)); \
    lua_pushvalue(L, idx); \
    lua_settable(L, LUA_REGISTRYINDEX); \
};
#define luaL_dropreg(L, p) \
{ \
    lua_pushlightuserdata(L, (void*)(p)); \
    lua_pushnil(L); \
    lua_settable(L, LUA_REGISTRYINDEX); \
};


#define CASE_FUNC(L, field, name, udatatype) \
    if (g_strcmp0((field), (name)) == 0) \
    { \
        lua_pushcfunction((L), name##_##udatatype); \
        return 1; \
    } \

#define CASE_NUM(L, field, name, value) \
    if (g_strcmp0((field), (name)) == 0) \
    { \
        lua_pushnumber((L), (value)); \
        return 1; \
    } \

#define CASE_BOOL(L, field, name, value) \
    if (g_strcmp0((field), (name)) == 0) \
    { \
        lua_pushboolean((L), (value)); \
        return 1; \
    } \

#define CASE_STR(L, field, name, value) \
    if (g_strcmp0((field), (name)) == 0) \
    { \
        lua_pushboolean((L), (value)); \
        return 1; \
    } \

void luaL_setfuncs (lua_State*, const luaL_Reg*, int);
void luaL_newlib(lua_State*, const luaL_Reg*, const char*);
