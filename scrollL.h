#pragma once

#include "conf.h"

typedef struct scrollL
{
    GtkScrolledWindow* scroll;
    GtkImage*          image;
}
scrollL;

int luaopen_scrollL(lua_State*, const gchar*);
