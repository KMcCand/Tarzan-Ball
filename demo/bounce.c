#include "sdl_wrapper.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "color.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// Properties of Star
const int NUM_POINTS = 5;
const size_t STAR_RADIUS = 50;
const rgb_color_t COLOR = {0.0, 1.0, 1.0};
const double STAR_RATIO = 2.3;
const double STAR_MASS = 10.0;
const double VEL_X = 800.0;
const double VEL_Y = 600.0;
const double ROTATION =  2.4;

// Properties of the screen
const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;


/**
 * Creates and returns a list_t of points that form a star with radius
 * STAR_RADIUS, number of points NUM_POINTS, and ratio between the radius
 * of the inner to outer points STAR_RATIO
 * 
 * @return a pointer to a list_t of points of a star
 */
list_t *star_points() {
    list_t *ret = list_init(2 * NUM_POINTS, (free_func_t) vec_free);
    double center_x = (MAX_X + MIN_X) / 2.0;
    double center_y = (MAX_Y + MIN_Y) / 2.0;

    for (int i = 0; i < 2 * NUM_POINTS; i++) {
        vector_t *vec = malloc(sizeof(vector_t));
        double angle = M_PI * i / NUM_POINTS;
        if (i % 2 == 0) {
            *vec = (vector_t) {center_x + STAR_RADIUS * cos(angle), center_y + STAR_RADIUS * sin(angle)};
        }
        else {
            *vec = (vector_t) {center_x + STAR_RADIUS / STAR_RATIO * cos(angle), 
                               center_y + STAR_RADIUS / STAR_RATIO * sin(angle)};
        }
        list_add(ret, vec);
    }
    return ret;
}

/**
 * Creates and returns a pointer to a body_t that represents a star, with properties
 * determined by the constants at the top of bounce.c
 * 
 * @return a pointer to a body_t representing a star
 */
body_t *create_star() {
    body_t *new_star = body_init(star_points(), STAR_MASS, COLOR);
    body_set_passive_rotation(new_star, ROTATION);
    body_set_velocity(new_star, (vector_t) {VEL_X, VEL_Y});

    return new_star;
}

/**
 * Checks every point of every body in scene to make sure it is still in the window.
 * If any point of any body has gone over any wall, reverse the body's x or y velocity
 * to create a bounce off of the wall.
 * 
 * @param scene a pointer to a scene_t returned by scene_init
 */
void check_boundaries(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i ++) {
        int changed_x = 0;
        int changed_y = 0;
        body_t *body = scene_get_body(scene, i);
        vector_t velocity = body_get_velocity(body);

        for (size_t j = 0; j < list_size(body_get_shape(body)); j++) {
            vector_t *current_point = list_get(body_get_shape(body), j);

            if (current_point->x <= MIN_X && !changed_x) {
                body_set_velocity(body, (vector_t) {fabs(velocity.x), velocity.y});
                changed_x = true;
            }
            else if (current_point->x >= MAX_X && !changed_x) {
                body_set_velocity(body, (vector_t) {-fabs(velocity.x), velocity.y});
                changed_x = true;
            }
            if (current_point->y <= MIN_Y && !changed_y) {
                body_set_velocity(body, (vector_t) {velocity.x, fabs(velocity.y)});
                changed_y = true;
            }
            else if (current_point->y >= MAX_Y && !changed_y) {
                body_set_velocity(body, (vector_t) {velocity.x, -fabs(velocity.y)});
                changed_y = true;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    vector_t min = {MIN_X, MIN_Y};
    vector_t max = {MAX_X, MAX_Y};
    sdl_init(min, max);

    scene_t *bounce_scene = scene_init();
    scene_add_body(bounce_scene, create_star());
    
    while (!sdl_is_done(bounce_scene)) {
        double dt = time_since_last_tick();
        check_boundaries(bounce_scene);
        scene_tick(bounce_scene, dt);
        sdl_clear();
        sdl_draw_polygon(body_get_shape(scene_get_body(bounce_scene, 0)), COLOR);
        sdl_show();
    }

    scene_free(bounce_scene);
}
