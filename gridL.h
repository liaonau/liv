#pragma once

#include "conf.h"

typedef struct gridL
{
    GtkScrolledWindow* scroll;
    GtkGrid*           grid;
    GPtrArray*         frames;
    gint               size;
    gint               rows;
    gint               cols;
}
gridL;

int luaopen_gridL(lua_State*, const gchar*);
