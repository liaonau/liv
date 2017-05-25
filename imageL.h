#pragma once

#include "conf.h"

typedef struct imageL
{
    gchar*           path;

    GdkPixbufFormat* format;
    GdkPixbuf*       pxb;

    gint             native_width;
    gint             native_height;

    guint8           state;
    gint             width;
    gint             height;
}
imageL;

GdkPixbuf* image_get_pixbuf(imageL*);
int luaopen_imageL(lua_State*);

GdkPixbuf* BROKENpxb;
