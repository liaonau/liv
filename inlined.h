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

#define BROKEN_NAME   "/resources/broken.png"
#define DEFERRED_NAME "/resources/deferred.gif"
#define LUA_NAME      "/resources/lua.ico"
#define APP_NAME      "/resources/app.ico"
#define LOADING_NAME  "/resources/loading.gif"

#define PNG_FORMAT_NAME "png"

extern GdkPixbuf*          BROKENpxb;
extern GdkPixbuf*          LUApxb;
extern GdkPixbufAnimation* DEFERREDpxb;
extern GdkPixbuf*          APPpxb;
extern GdkPixbuf*          LOADINGpxb;

extern GdkPixbufFormat*    PNGformat;

gboolean init_resources(void);
