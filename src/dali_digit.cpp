/****************************************************************************
 *   DaliClock by (c) 2021 Marcio Teixeira                               *
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
#include "gfx/CompositeGraphics.h"
#include "dali_digit.h"

const dali_digit_t &DaliDigit::getDigitInfo(char c) {
    const dali_digit_t *digits[] = {
        &DIGIT_INFO(slash),
        &DIGIT_INFO(zero),
        &DIGIT_INFO(one),
        &DIGIT_INFO(two),
        &DIGIT_INFO(three),
        &DIGIT_INFO(four),
        &DIGIT_INFO(five),
        &DIGIT_INFO(six),
        &DIGIT_INFO(seven),
        &DIGIT_INFO(eight),
        &DIGIT_INFO(nine),
        &DIGIT_INFO(colon)
    };
    return *digits[(c - '/') % 11];
}

void DaliDigit::init(char c1, char c2, uint8_t blend) {
    const dali_digit_t &d1 = getDigitInfo(c1);
    const dali_digit_t &d2 = getDigitInfo(c2);
    height      = max(d1.height, d2.height);
    width       = max(d1.width, d2.width);
    _linestride = max(d1.linestride, d2.linestride);
    _rle_1      = d1.packed_rle;
    _rle_2      = d2.packed_rle;
    _blend      = blend;
    _row        = 0;
}

DaliDigit::DaliDigit(char c) {
    init(c, c, 0);
}

DaliDigit::DaliDigit(char c1, char c2, uint8_t blend) {
    init(c1, c2, blend);
}

DaliDigit::DaliDigit(float digit, uint8_t wrap) {
    const uint8_t f = floor(digit);
    const uint8_t c = ceil(digit);
    init('0' + f, '0' + (f == wrap ?  0 : c), min(255,int((digit - f)*512)));
}

void DaliDigit::unpack_rle_to_graphics(CompositeGraphics &g, const uint8_t *src, int x, int y, uint8_t color) {
    uint8_t len = 4;
    uint8_t n;
    do {
        // Skip background pixels
        x += *src++;
        // Draw foreground pixels
        n = *src++;
        while(n--) g.dot(x++, y, color);
        len -= 2;
    } while(len != 0);
}

void DaliDigit::rle_to_segment_endpoints(uint8_t rle[]) {
    if(rle[3] != 0) {
        rle[1] = rle[0] + rle[1];
        rle[2] = rle[1] + rle[2];
        rle[3] = rle[2] + rle[3];
    } else {
        rle[1] = rle[0] + rle[1];
        rle[2] = rle[0];
        rle[3] = rle[1];
    }
}

void DaliDigit::segment_endpoints_to_rle(uint8_t rle[]) {
    if(rle[2] > rle[1]) {
        rle[3] -= rle[2];
        rle[2] -= rle[1];
        rle[1] -= rle[0];
    } else {
        rle[1]  = rle[3] - rle[0];
        rle[2]  = 0;
        rle[3]  = 0;
    }
}

void DaliDigit::blend_rle(uint8_t start[4], uint8_t final[], uint8_t blend) {
    rle_to_segment_endpoints(start);
    rle_to_segment_endpoints(final);
    for(uint8_t i = 0; i < 4; i++) {
        final[i] = ((256-blend) * start[i] + blend * final[i]) / 256;
    }
    segment_endpoints_to_rle(final);
}

bool DaliDigit::draw_row(CompositeGraphics &g, int x, int y, uint8_t color) {
    if(_row++ == height) return false;
    uint8_t start[4], final[4];
    memcpy(start, _rle_1, 4); _rle_1 += 4;
    memcpy(final, _rle_2, 4); _rle_2 += 4;
    blend_rle(start, final, _blend);
    unpack_rle_to_graphics(g, final, x, y, color);
    return true;
}



