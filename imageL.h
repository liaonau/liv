#pragma once

#include "conf.h"

typedef struct imageL
{
    GdkPixbuf*       pxb;
    gchar*           path;
    gboolean         memorize;
    gboolean         broken;
    GdkPixbufFormat* format;
    guint8           state;
    gint             width;
    gint             height;
    gint             native_width;
    gint             native_height;
}
imageL;

GdkPixbuf* image_get_pixbuf(imageL*);
int luaopen_imageL(lua_State*, const gchar*);

GdkPixbuf* BROKENpxb;
