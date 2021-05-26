#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
    float r;
    float g;
    float b;
} rgb_color_t;

/**
 * Initialize a color with given RGB values
 * 
 * @param r_val red hex value
 * @param g_val green hex value
 * @param b_val blue hex value
 */
rgb_color_t color_init(float r_val, float g_val, float b_val);

/**
 * Returns red value of given color pointer
 * 
 * @param color pointer to a color returned from color_init
 */
float color_get_r(rgb_color_t *color);

/**
 * Returns green value of given color pointer
 * 
 * @param color pointer to a color returned from color_init
 */
float color_get_g(rgb_color_t *color);

/**
 * Returns blue value of given color pointer
 * 
 * @param color pointer to a color returned from color_init
 */
float color_get_b(rgb_color_t *color);

/**
 * Sets the r value of the color at color to new_r
 * 
 * @param color pointer to a color returned from color_init
 * @param new_r the r value to change to
 */
void color_set_r(rgb_color_t *color, float new_r);

/**
 * Sets the g value of the color at color to new_g
 * 
 * @param color pointer to a color returned from color_init
 * @param new_g the g value to change to
 */
void color_set_g(rgb_color_t *color, float new_g);

/**
 * Sets the b value of the color at color to new_b
 * 
 * @param color pointer to a color returned from color_init
 * @param new_b the b value to change to
 */
void color_set_b(rgb_color_t *color, float new_b);

/**
 * free color information given pointer
 * 
 * @param color pointer to a color returned from color_init
 */
void color_free(rgb_color_t *color);

#endif // #ifndef __COLOR_H__
