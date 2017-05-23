#pragma once

#include "conf.h"

GdkPixbuf* image_get_pixbuf(imageL*);
int luaopen_imageL(lua_State*, const gchar*);
