#include "util.h"
#include "conf.h"
#include "opts.h"

#include <string.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <stdlib.h>

/* Print error and exit with EXIT_FAILURE code. */
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

/* Print error message on stderr. */
void _warn(gint line, const gchar *fct, const gchar *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    gint atty = isatty(STDERR_FILENO);
    if (atty) g_fprintf(stderr, ANSI_COLOR_YELLOW);
    g_fprintf(stderr, "E: %s: %s:%d: ", APPNAME, fct, line);
    g_vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (atty) g_fprintf(stderr, ANSI_COLOR_RESET);
    g_fprintf(stderr, "\n");
}
