#include "util.h"
#include "conf.h"

#include <string.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <stdlib.h>

void _fatal(gint line, const gchar *fct, const gchar *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    gint atty = isatty(STDERR_FILENO);
    if (atty) g_fprintf(stderr, ANSI_COLOR_BG_RED);
    g_fprintf(stderr, "E: %s: %s:%d: ", APPNAME, fct, line);
    g_vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (atty) g_fprintf(stderr, ANSI_COLOR_RESET);
    g_fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}
void _warn(gint line, const gchar *fct, const gchar *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    gint atty = isatty(STDERR_FILENO);
    if (atty) g_fprintf(stderr, ANSI_COLOR_BG_YELLOW);
    g_fprintf(stderr, "W: %s: %s:%d: ", APPNAME, fct, line);
    g_vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (atty) g_fprintf(stderr, ANSI_COLOR_RESET);
    g_fprintf(stderr, "\n");
}
void _info(gint line, const gchar *fct, const gchar *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    gint atty = isatty(STDERR_FILENO);
    if (atty) g_fprintf(stderr, ANSI_COLOR_MAGENTA);
    g_fprintf(stderr, "I: %s: %s:%d: ", APPNAME, fct, line);
    g_vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (atty) g_fprintf(stderr, ANSI_COLOR_RESET);
    g_fprintf(stderr, "\n");
}
