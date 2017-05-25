#pragma once

#include "conf.h"

#define PNG_FORMAT_NAME "png"

GdkPixbuf*       BROKENpxb;
GdkPixbufFormat* PNGformat;

gboolean init_inlined_objects(void);
