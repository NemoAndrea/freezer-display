#pragma once
#include "lvgl.h"

// get a grayscale colour that we can use on the 4-bit dispaly
lv_color_t get_grayscale_color(uint8_t value){
    assert(value < 16);

    lv_color_t grayscale;

    // we hift the 0bXXXX value to 0bXXXX0 000000 00000 as we only use
    // the last 4 bits of the RRRRR GGGGGG BBBBB data
    uint16_t rawcolor = (uint16_t)value << 12;
    grayscale.full = rawcolor;

    return grayscale;
}