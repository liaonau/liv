#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

#define UDATA_IMAGEL  "imageL"
#define UDATA_GRIDL   "gridL"
#define UDATA_SCROLLL "scrollL"
#define UDATA_APPL    "appL"

#define LIB_IMAGEL  "image"
#define LIB_GRIDL   "grid"
#define LIB_SCROLLL "scroll"
#define LIB_APPL    "app"

typedef struct appL
{
}
appL;
typedef struct gridL
{
    GtkScrolledWindow* scroll;
    GtkGrid*           grid;
    GPtrArray*         frames;
    gint               size;
    gint               rows;
    gint               cols;
}
gridL;
typedef struct scrollL
{
    GtkScrolledWindow* scroll;
    GtkImage*          image;
}
scrollL;
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

GtkWindow* window;
GtkFrame*  content;

GtkBox*    statusbox;
GtkLabel*  status_left;
GtkLabel*  status_right;

