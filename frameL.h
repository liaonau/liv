#pragma once

#include "conf.h"

typedef struct frameL
{
    GtkFrame* frame;
    GtkImage* image;
}
frameL;

int luaopen_frameL(lua_State*, const gchar*);
