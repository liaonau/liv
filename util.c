/*
 * Copyright © 2017 Roman Leonov <rliaonau@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
