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
#include "imageL.h"

typedef struct idle_load_t
{
    gint       ref;
    imageL*    i;
    GdkPixbuf* pxb;
    lua_State* L;
    GtkImage*  image;
}
idle_load_t;

typedef void       (*idle_pixbuf_fail_cb)(imageL*);
typedef GdkPixbuf* (*idle_pixbuf_load_cb)(imageL*);
typedef struct idle_pixbuf_t
{
    imageL*             i;
    GdkPixbuf*          pxb;
    idle_pixbuf_load_cb load_cb;
    imageL*             load_source;
    idle_pixbuf_fail_cb fail_cb;
}
idle_pixbuf_t;


void idle_load(lua_State*, GtkImage*, imageL*, gboolean);
void idle_pixbuf(imageL* i, idle_pixbuf_load_cb, imageL*, idle_pixbuf_fail_cb);
