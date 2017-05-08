#pragma once

#include <lua.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

typedef GdkPoint point;

typedef struct
{
    gchar*    filename;
    GtkImage* thumb;
    point     size;
    double    aspect;
    gboolean  marked;
    guint     index;
}
image_t;


typedef struct
{
    gint  max_thumb_size;
    point window_size;
}
conf_t;

conf_t conf;

gint pointer;
GArray* files;

gchar** infiles;
gchar*  execpath;

lua_State* L;
