#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

void luaL_setfuncs (lua_State*, const luaL_Reg*, int);
void luaL_newlib(lua_State*, const luaL_Reg*, const char*);
