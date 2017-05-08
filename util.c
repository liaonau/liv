#include "util.h"
#include "conf.h"

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

keypress_t* keyval_to_keypress(GdkEventKey* ev)
{
    guint transformed_keyval = ev->keyval;
    gdk_keymap_translate_keyboard_state(
            gdk_keymap_get_default(),
            ev->hardware_keycode,
            ev->state,
            0, /* default group */
            &transformed_keyval, NULL, NULL, NULL);
    gchar ucs[32];
    guint ulen;
    guint32 ukval = gdk_keyval_to_unicode(transformed_keyval);
    if (g_unichar_isgraph(ukval))
    {
        ulen = g_unichar_to_utf8(ukval, ucs);
        ucs[ulen] = 0;
    }
    else
    {
        gchar *p = gdk_keyval_name(transformed_keyval);
        guint len = strlen(p);
        for (gint i = 0; *p; p++, i++)
            ucs[i] = *p;
        ucs[len] = 0;
    }

    keypress_t* keypress = g_malloc(sizeof(keypress_t));
    keypress->transformed_keyval = transformed_keyval;
    strcpy(keypress->keyname, ucs);
    keypress->keyval = ev->keyval;
    return keypress;
}

