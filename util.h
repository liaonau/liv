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

/* ANSI term color codes */
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

#define ANSI_COLOR_BG_RED    "\x1b[41m"
#define ANSI_COLOR_BG_YELLOW "\x1b[43m"

void _fatal(gint, const gchar *, const gchar *, ...);
#define fatal(string, ...) _fatal(__LINE__, __func__, string, ##__VA_ARGS__)

#define warn(string, ...) _warn(__LINE__, __func__, string, ##__VA_ARGS__)
void _warn(gint, const gchar *, const gchar *, ...);

#define info(string, ...) _info(__LINE__, __func__, string, ##__VA_ARGS__)
void _info(gint, const gchar *, const gchar *, ...);

