#pragma once

#include <glib.h>

#define APPNAME "liv"

typedef struct
{
    /*gboolean onecolumn;*/
    /*gboolean checkbox;*/
    gboolean numbers;
    /*gboolean underline;*/
    /*gboolean color;*/
    /*gboolean radiobox;*/
    /*gboolean initial;*/
    /*gboolean fullattr;*/
    /*gboolean whitelines;*/
    /*gchar*   foreground;*/
    gchar*   background;

    gchar*   execpath;

    gchar**  infiles;
} conf_t;

conf_t conf;
