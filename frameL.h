#pragma once

#include "conf.h"
#include "imageL.h"

typedef struct frameL
{
    GtkFrame* frame;
    GtkImage* image;
    gint      ref;
}
frameL;

int luaopen_frameL(lua_State*);
