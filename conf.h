#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

#define UDATA_IMAGEL "«image»"
#define UDATA_GRIDL  "«grid»"
#define UDATA_APPL   "«app»"

#define LIB_IMAGEL "image"
#define LIB_GRIDL  "grid"
#define LIB_APPL   "app"

typedef struct appL
{
}
appL;

typedef struct gridL
{
    GtkGrid* grid;
}
gridL;

typedef struct imageL
{
    const gchar* path;
    gboolean     broken;
    GdkPixbuf*   originalpxb;
    GtkFrame*    frame;
    GtkImage*    image;
    guint8       state;
}
imageL;

lua_State* L;

GtkWindow* window;
GtkWidget* scroll;

