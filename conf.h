#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

#define UDATA_IMAGEL "«image»"
#define UDATA_GRIDL  "«grid»"
#define UDATA_FRAMEL "«frame»"
#define UDATA_APPL   "«app»"

#define LIB_IMAGEL "image"
#define LIB_GRIDL  "grid"
#define LIB_FRAMEL "frame"
#define LIB_APPL   "app"

typedef struct appL
{
}
appL;

typedef struct thumb_t
{
    GtkFrame* frame;
    GtkImage* image;
} thumb_t;

typedef struct gridL
{
    GtkGrid*   grid;
    GPtrArray* thumbs;

    gint left;
    gint top;
}
gridL;

typedef struct frameL
{
    GtkImage*          image;
    GtkScrolledWindow* scroll;
}
frameL;

typedef struct imageL
{
    gchar*     path;
    gboolean   broken;
    GdkPixbuf* nativepxb;
    GdkPixbuf* pxb;
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

