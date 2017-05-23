#pragma once

#include "conf.h"

void idle_load(lua_State*, GtkImage*, imageL*, gboolean);
void idle_pixbuf(imageL* i, idle_pixbuf_load_cb, imageL*, idle_pixbuf_fail_cb);
