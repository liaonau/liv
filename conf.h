#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

#define UDATA_IMAGEL  "«image»"
#define UDATA_GRIDL   "«grid»"
#define UDATA_SCROLLL "«scroll»"
#define UDATA_APPL    "«app»"

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

    gint left;
    gint top;
}
gridL;

typedef struct scrollL
{
    GtkScrolledWindow* scroll;
}
scrollL;

typedef struct imageL
{
    gchar*     path;
    gboolean   broken;
    GdkPixbuf* pxb;
    GtkImage*  image;
    guint8     state;
}
imageL;

lua_State* L;

GtkWindow* window;
GtkBox*    mainbox;
GtkBox*    content;

GtkBox*    statusbox;
GtkLabel*  status_left;
GtkLabel*  status_right;

