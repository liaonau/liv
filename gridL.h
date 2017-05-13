#pragma once

#include "conf.h"

typedef struct grid_count_t
{
    GtkGrid*   grid;
    gint       left;
    gint       top;
} grid_count_t;

int luaopen_gridL(lua_State*, const gchar*);
