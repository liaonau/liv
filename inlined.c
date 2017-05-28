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

#include "inlined.h"
#include "resources.h"

#define FAIL_ON_ERROR(e) \
{                        \
    if (e != NULL)       \
    {                    \
        g_error_free(e); \
        return FALSE;    \
    }                    \
}

gboolean init_resources(void)
{
    BROKENpxb   = NULL;
    DEFERREDpxb = NULL;
    PNGformat   = NULL;

    GError* error = NULL;
    BROKENpxb = gdk_pixbuf_new_from_resource(BROKEN_NAME, &error);
    FAIL_ON_ERROR(error);
    DEFERREDpxb = gdk_pixbuf_animation_new_from_resource(DEFERRED_NAME, &error);
    FAIL_ON_ERROR(error);
    if (!GDK_IS_PIXBUF(BROKENpxb))
        return FALSE;
    if (!GDK_IS_PIXBUF_ANIMATION(DEFERREDpxb))
        return FALSE;

    GSList* formats = gdk_pixbuf_get_formats();
    GSList* f = formats;
    while(f)
    {
        GdkPixbufFormat* format = (GdkPixbufFormat*)f->data;
        if (g_strcmp0(gdk_pixbuf_format_get_name(format), PNG_FORMAT_NAME) == 0)
        {
            PNGformat = format;
            break;
        }
        f = f->next;
    }
    g_slist_free(formats);
    if (!PNGformat)
        return FALSE;

    return TRUE;
}
