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

#pragma once

#include "conf.h"

#define IMAGE_CHANGED_SIGNAL "image-changed"

typedef struct imageL
{
    /*RO*/
    gchar*           path;
    GdkPixbufFormat* format;
    gint             native_width;
    gint             native_height;
    GdkPixbuf*       pxb;

    /*RW*/
    guint8           state;
    gint             width;
    gint             height;

    GObject*         emitter;
    GRWLock          lock;
}
imageL;

GdkPixbuf* image_get_scaled_pixbuf(imageL*, gint, gint);
GdkPixbuf* image_get_stated_pixbuf(imageL*, GdkPixbuf*, guint8);
GdkPixbuf* image_get_current_pixbuf(imageL*);

int luaopen_imageL(lua_State*);
