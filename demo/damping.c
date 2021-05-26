#include "forces.h"
#include "scene.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "color.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Constants for the window
const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;

// Constants for the visible dots
const size_t DOT_NUM_POINTS = 20;
const double DOT_RADIUS = 8;
const double DOT_MASS = 6;
const vector_t DOT_MAX_VELOCITY = {0, 300};
const double DOT_SPRING_CONSTANT = .10;
const double DOT_DAMPING_CONSTANT = .005;
const rgb_color_t DOT_STARTING_COLOR = {0.0, 0.5, 0.5};
const float DOT_COLOR_INCREMENT = 0.00001;
const double DOT_FRACTION_OF_SINE_WAVE_PERIOD = 0.5;

// Constants for the invisible dots
const vector_t INVISIBLE_DOT_VELOCITY = {0, 0};
const rgb_color_t INVISIBLE_DOT_COLOR = {1, 1, 1};
const size_t INVISIBLE_DOT_NPOINTS = 3;
const double INVISIBLE_DOT_RADIUS = 0.01;


/**
 * Returns a pointer to a body_t that represents a circle at center with the given parameters.
 * 
 * @param center the center of the circle
 * @param starting_vel the circle's initial velocity
 * @param color the circle's color
 * @param num_points the number of points used to draw the circle
 * @param radius the circle's radius
 * @param mass the circle's mass
 * @return a pointer to a body_t for the circle with all of these properties.
 */
body_t *create_dot_body(vector_t center, vector_t starting_vel, rgb_color_t color, size_t num_points, double radius, double mass) {
    list_t *dot_points = list_init(num_points, (free_func_t) vec_free);

    for (size_t i = 0; i < num_points; i++) {
        double angle = i * 2.0 * M_PI / num_points;

        vector_t *v = malloc(sizeof(*v));
        *v = (vector_t) {center.x + radius * cos(angle), center.y + radius * sin(angle)};
        list_add(dot_points, v);
    }

    body_t *dot_body = body_init(dot_points, mass, color);
    body_set_velocity(dot_body, starting_vel);
    return dot_body;
}

/**
 * Returns a pointer to a body_t that represents a circle at center with the given parameters.
 * 
 * @param center the center of the circle
 * @param starting_vel the circle's initial velocity
 * @param color the circle's color
 * @param num_points the number of points used to draw the circle
 * @param radius the circle's radius
 * @param mass the circle's mass
 * @return a pointer to a body_t for the circle with all of these properties.
 */
void create_all_dots(scene_t *scene) {
    double left = MIN_X + DOT_RADIUS;
    double right = MAX_X + DOT_RADIUS;
    rgb_color_t *color = malloc(sizeof(*color));
    *color = DOT_STARTING_COLOR;

    for (size_t x = left; x < right; x += 2 * DOT_RADIUS) {
        vector_t current_center = {x, MAX_Y / 2};
        
        double color_increment = DOT_COLOR_INCREMENT * x;
        color_set_r(color, fmod(color_get_r(color) + color_increment, 1.0));
        color_set_g(color, fmod(color_get_g(color) + color_increment, 1.0));
        color_set_b(color, fmod(color_get_b(color) + color_increment, 1.0));
 
        double angle = (x - left) / (right - left) * 2 * M_PI * DOT_FRACTION_OF_SINE_WAVE_PERIOD;
        scene_add_body(scene, create_dot_body(current_center, vec_multiply(cos(angle), DOT_MAX_VELOCITY), *color,
                    DOT_NUM_POINTS, DOT_RADIUS, DOT_MASS));
        scene_add_body(scene, create_dot_body(current_center, INVISIBLE_DOT_VELOCITY, INVISIBLE_DOT_COLOR,
                    INVISIBLE_DOT_NPOINTS, INVISIBLE_DOT_RADIUS, INFINITY));
    }
}

/**
 * Updates the spring forces between all bodies in the scene_t pointed to by scene.
 * 
 * @param scene a scene to make spring forces on
 */
void update_spring(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene) - 1; i+=2) {
        create_spring(scene, DOT_SPRING_CONSTANT, scene_get_body(scene, i), scene_get_body(scene, i + 1));
    }
}

/**
 * Updates the drag forces for all bodies in the scene_t pointed to by scene.
 * 
 * @param scene a scene to make drag forces on
 */
void update_drag(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        create_drag(scene, DOT_DAMPING_CONSTANT, scene_get_body(scene, i));
    }
}

int main(int argc, char *argv[]){
    scene_t *scene = scene_init();
    sdl_init((vector_t) {MIN_X, MIN_Y}, (vector_t) {MAX_X, MAX_Y});
    create_all_dots(scene);

    while(!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        update_spring(scene);
        update_drag(scene);
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }

    scene_free(scene);
}
