#include "color.h"
#include <stdlib.h>

rgb_color_t color_init(float r_val, float g_val, float b_val) {
    return (rgb_color_t) {r_val, g_val, b_val};
}

float color_get_r(rgb_color_t *color) {
    return color->r;
}

float color_get_g(rgb_color_t *color) {
    return color->g;
}

float color_get_b(rgb_color_t *color) {
    return color->b;
}

void color_set_r(rgb_color_t *color, float new_r) {
    color->r = new_r;
}

void color_set_g(rgb_color_t *color, float new_g) {
    color->g = new_g;
}

void color_set_b(rgb_color_t *color, float new_b) {
    color->b = new_b;
}

void color_free(rgb_color_t *color) {
    free(color);
}
