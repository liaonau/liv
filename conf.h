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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <glib.h>
#include <gtk/gtk.h>

#define APPNAME "liv"

#define APP     "app"
#define FRAME   "frame"
#define IMAGE   "image"
#define GRID    "grid"

GtkWindow*         window;
GtkScrolledWindow* scroll;
GtkAdjustment*     hadj;
GtkAdjustment*     vadj;
gint               displayref;

GtkBox*    statusbox;
GtkLabel*  status_left;
GtkLabel*  status_right;

