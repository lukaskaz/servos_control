#ifndef __RGB_API__
#define __RGB_API__

#include "types.hpp"

#define RGB_BRIGHT_MAX  255U

typedef enum {
    RGB_COL_FIRST = (-1),
    RGB_NONE,
    RGB_WHITE,
    RGB_RED,
    RGB_GREEN,
    RGB_BLUE,
    RGB_COL_LAST
} rgb_color_t;

typedef enum {
    RGB_FADE_UP = 0,
    RGB_FADE_DOWN,
} rgb_dimm_t;

int8_t rgb_set_color(rgb_color_t, uint8_t);
int8_t rgb_dimm_color(rgb_color_t, rgb_dimm_t, uint32_t);

#endif

