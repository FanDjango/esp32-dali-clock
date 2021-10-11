/****************************************************************************
 *   DaliClock by (c) 2021 Marcio Teixeira                                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   To view a copy of the GNU General Public License, go to the following  *
 *   location: <http://www.gnu.org/licenses/>.                              *
 ****************************************************************************/

#include <Arduino.h>
#include "../dali_config.h"
#include "gfx/CompositeGraphics.h"
#include "dali_sparkle.h"

void DaliSparkle::locate(CompositeGraphics &g, float t, int x, int y, int w, int h, char mask_color) {
    if(t > sparkle_duration) {
        do {
            // Random position
            _x = random(x, x+w);
            _y = random(y, y+h);
            // Find nearest edge
            const bool onMask = g.get(_x,_y) == mask_color;
            while(onMask == (g.get(_x,_y) == mask_color)) {
                _x++;
                if(_x == display_width) break;
            }
        } while (_x == display_width);
    }
}

void DaliSparkle::draw(CompositeGraphics &g, float t) {
    if(t > 0 && t < sparkle_duration) {
        const int r = sparkle_size * sin(t/sparkle_duration*2*PI);
        // Vary the color and lengths of the diagonal and horizontal elements
        const char c1 = _x&1 ? 0x0F : 0x09;
        const char c2 = _x&1 ? 0x09 : 0x0F;
        const int  r1 = _y&1 ? r    : r/2;
        const int  r2 = _y&1 ? r/2  : r;
        const bool h_1st = _y&2;
        // Draw the sparkle.
        if(h_1st == (r>0)) {
            g.line(_x-r1,_y   ,_x+r1,_y  ,c1);
            g.line(_x   ,_y-r2,_x   ,_y+r2,c2);
        } else {
            g.line(_x-r1,_y+r1,_x+r1,_y-r1,c1);
            g.line(_x-r2,_y-r2,_x+r2,_y+r2,c2);
        }
    }
}
