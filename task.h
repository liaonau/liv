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
    imageL*    i;
    gulong     time;
    callback_t func;
}
task_frame_t;

typedef struct task_pixbuf_t
{
    imageL*    i;
    GdkPixbuf* pxb;
}
task_pixbuf_t;

void task_frame_from_image_pixbuf(frameL*, imageL*, gulong, callback_t);
void task_image_pixbuf_from_file(imageL*);
void task_image_pixbuf_from_pixbuf(imageL*, GdkPixbuf*);
