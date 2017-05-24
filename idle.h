#pragma once

#include "conf.h"
#include "imageL.h"

typedef struct idle_load_t
{
    gint       ref;
    imageL*    i;
    GdkPixbuf* pxb;
    lua_State* L;
    GtkImage*  image;
}
idle_load_t;

typedef void       (*idle_pixbuf_fail_cb)(imageL*);
typedef GdkPixbuf* (*idle_pixbuf_load_cb)(imageL*);
typedef struct idle_pixbuf_t
{
    imageL*             i;
    GdkPixbuf*          pxb;
    idle_pixbuf_load_cb load_cb;
    imageL*             load_source;
    idle_pixbuf_fail_cb fail_cb;
}
idle_pixbuf_t;


void idle_load(lua_State*, GtkImage*, imageL*, gboolean);
void idle_pixbuf(imageL* i, idle_pixbuf_load_cb, imageL*, idle_pixbuf_fail_cb);
