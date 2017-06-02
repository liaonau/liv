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

#pragma once

#include "util.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define GET_ADJ(dir, type, field)                  \
    {                                              \
        gdouble val;                               \
        val = gtk_adjustment_get_##type(dir##adj); \
        lua_pushnumber(L, val);                    \
        lua_setfield(L, -2, field);                \
    }

#define SET_ADJ(num, dir, type, field)                \
    {                                                 \
        gdouble tmp, val;                             \
        tmp = gtk_adjustment_get_##type(dir##adj);    \
        lua_getfield(L, num, field);                  \
        val = luaL_optnumber(L, -1, tmp);             \
        lua_pop(L, 1);                                \
        if (tmp != val)                               \
            gtk_adjustment_set_##type(dir##adj, val); \
    }

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

void luaH_pcall(lua_State*, int, int);
