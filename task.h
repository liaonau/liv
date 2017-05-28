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

#include "conf.h"
#include "frameL.h"
#include "imageL.h"

typedef void (*callback_t)(frameL*, GdkPixbuf*);

typedef struct task_frame_t
{
    frameL*    f;
    gulong     time;

    imageL*    i;
    gint       width;
    gint       height;
    guint8     state;

    callback_t func;
}
task_frame_t;

typedef struct task_pixbuf_t
{
    imageL*    i;
    GdkPixbuf* pxb;
}
task_pixbuf_t;

typedef struct task_dump_t
{
    imageL*      i;
    const gchar* name;
    gchar*       path;
    gint         width;
    gint         height;
    guint8       state;

    lua_State*   L;
    gint         ref;
}
task_dump_t;

void task_frame_from_image_pixbuf(frameL*, imageL*, gulong, callback_t);
void task_image_pixbuf_from_file(imageL*);
void task_image_pixbuf_from_pixbuf(imageL*, GdkPixbuf*);
void task_dump_image(imageL*, const gchar*, const gchar*, lua_State*, gint);
