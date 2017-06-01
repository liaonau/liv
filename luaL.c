/*
 * Copyright © 2017 Roman Leonov <rliaonau@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "luaL.h"

void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup)
{
    luaL_checkstack(L, nup+1, "too many upvalues");
    for (; l->name != NULL; l++)
    {  /* fill the table with given functions */
        int i;
        lua_pushstring(L, l->name);
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(L, -(nup+1));
        lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        lua_settable(L, -(nup + 3));
    }
    lua_pop(L, nup);  /* remove upvalues */
}
void luaL_newlib(lua_State *L, const luaL_Reg *l, const char* n)
{
    lua_newtable(L);
    luaL_setfuncs(L,l,0);
    lua_setglobal(L, n);
}

static int luaH_traceback(lua_State *L)
{
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_replace(L, -2);
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    return 1;
}
void luaH_pcall(lua_State *L, int nargs, int nresults)
{
    lua_pushcfunction(L, luaH_traceback);
    lua_insert(L, - nargs - 2);
    int error_func_pos = lua_gettop(L) - nargs - 1;
    if (lua_pcall(L, nargs, nresults, -nargs - 2))
    {
        warn("%s", lua_tostring(L, -1));
        lua_pop(L, 2);
        return;
    }
    lua_remove(L, error_func_pos);
}

