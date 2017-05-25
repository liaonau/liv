#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

#define APP     "app"
#define FRAME   "frame"
#define IMAGE   "image"
#define GRID    "grid"

GtkWindow*         window;
GtkScrolledWindow* scroll;

GtkBox*    statusbox;
GtkLabel*  status_left;
GtkLabel*  status_right;

