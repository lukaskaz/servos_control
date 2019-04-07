#ifndef __RGB_API__
#define __RGB_API__

#include "types.hpp"

typedef enum {
    RGB_COL_FIRST = (-1),
    RGB_NONE,
    RGB_WHITE,
    RGB_RED,
    RGB_GREEN,
    RGB_BLUE,
    RGB_COL_LAST
} rgb_color_t;

int8_t rgb_set_color(rgb_color_t color, uint8_t bright);

#endif

