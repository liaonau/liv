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
    GtkGrid*   grid;
    GPtrArray* frames;
    gint       rows;
    gint       cols;
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
    gchar*     path;
    GdkPixbuf* pxb;
    guint8     state;
    gint       width;
    gint       height;
}
imageL;

lua_State* L;

GtkWindow* window;
GtkFrame*  content;

GtkBox*    statusbox;
GtkLabel*  status_left;
GtkLabel*  status_right;

