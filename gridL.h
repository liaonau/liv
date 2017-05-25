#pragma once

#include "conf.h"

typedef struct gridL
{
    GtkGrid*   grid;
}
gridL;

int luaopen_gridL(lua_State*);
