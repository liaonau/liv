#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

#define UDATA_FILEL  "fileL"
#define UDATA_IMAGEL "imageL"
#define UDATA_GRIDL  "gridL"

#define LIB_FILEL  "file"
#define LIB_IMAGEL "image"
#define LIB_GRIDL  "grid"

typedef struct gridL
{
    GtkGrid* grid;
}
gridL;

typedef struct imageL
{
    GtkImage*  image;
    GdkPixbuf* pixbuf;
}
imageL;

typedef struct fileL
{
    gchar*     path;
    GdkPixbuf* pixbuf;
}
fileL;

lua_State* L;

GtkWindow* window;
GtkWidget* swindow;

