#pragma once
#include "conf.h"
#include "conf.h"

/* ANSI term color codes */
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

#define ANSI_COLOR_BG_RED  "\x1b[41m"

typedef struct
{
    gchar keyname[32];
    guint transformed_keyval;
    guint keyval;
}
keypress_t;

void _fatal(gint, const gchar *, const gchar *, ...);
#define fatal(string, ...) _fatal(__LINE__, __func__, string, ##__VA_ARGS__)

#define warn(string, ...) _warn(__LINE__, __func__, string, ##__VA_ARGS__)
void _warn(gint, const gchar *, const gchar *, ...);

keypress_t* keyval_to_keypress(GdkEventKey*);
