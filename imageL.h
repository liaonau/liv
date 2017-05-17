#pragma once

#include "conf.h"

GdkPixbuf* image_create_pixbuf(imageL*);
int luaopen_imageL(lua_State*, const gchar*);
