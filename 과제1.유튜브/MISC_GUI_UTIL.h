/*
 * Copyright (C) 2025 Lee Hackseung
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Copyleft License)
 * as published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MISC_GUI_UTIL_H__
#define __MISC_GUI_UTIL_H__

bool misc_gui_util_init(void);
void misc_gui_util_term(void);

void misc_util_draw_circle(Gdiplus::Bitmap* bitMap, int xc, int yc, int r, int th, UINT32 color);
void misc_util_draw_fill_circle(Gdiplus::Bitmap* bitMap, int xc, int yc, int r, UINT32 color);

#endif // !__MISC_GUI_UTIL_H__
