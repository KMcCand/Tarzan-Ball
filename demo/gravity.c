#include "sdl_wrapper.h"
#include "polygon.h"
#include "color.h"
#include "list.h"
#include "vector.h"
#include "scene.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Star drawing and movement constants
const int SIDES = 5;
const size_t SIZE = 50;
const rgb_color_t COLOR = {0.0, 1.0, 1.0};
const double ROTATION = 5 * M_PI;
const double VEL_X = 300.0;
const double GRAV_Y = -2000.0;
const double ELAST_MIN = 0.85;
const double ELAST_MAX = 1;
const int STARTING_SIDES = 2;
const double TIME_BETWEEN_STARS = 0.8;

// Frame properties
const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;
const double ENTRY_X = 0.0;
const double ENTRY_Y = 400.0;
const int LIST_STARTING_SIZE = 5;

/**
 * Creates an n-sided star with radius size centered at center.
 * 
 * @param n the number of STARTING_SIDES of the star
 * @param size the outer radius of the star
 * @param center the initial center of the star
 * @return a pointer to a vec_list_t representing the n points of the star
 */
list_t *create_star(int n, int size, vector_t center) {
    list_t *ret = list_init(2 * n, (free_func_t) vec_free);
    assert(ret != NULL && "Could not allocate memory for vec_list_t for star");

    // Set Golden Ratio between inner and outer radii of stars
    double ratio = (3 + sqrt(5))/2;

    for(int i = 0; i < 2 * n; i++) {
        vector_t *vec = malloc(sizeof(vector_t));
        assert(vec != NULL && "Could not allocate memory for vector_t for point of star");

        double dx = size * cos(M_PI * i / n + M_PI / 2);
        double dy = size * sin(M_PI * i / n + M_PI / 2);
        if (i % 2 == 0) {
            *vec = (vector_t) {center.x + dx, center.y + dy};
        }
        else {
            *vec = (vector_t) {center.x + dx / ratio, center.y + dy / ratio};
        }
        list_add_front(ret, vec);
    }
    return ret;
}

/**
 * Negates the y velocity of the polygon pointed to by poly if it has crossed the
 * minimum y value of min.
 * 
 * @param poly a pointer to a polygon_t star
 * @param min the vector_t bottom left corner of the screen
 */
void check_bounce(body_t *poly, vector_t min) {
    list_t *points = body_get_shape(poly);
    vector_t velocity = body_get_velocity(poly);

    for (size_t i = 0; i < list_size(points); i++) {
        if (((vector_t *) list_get(points, i))->y <= min.y && velocity.y < 0) {
            body_set_velocity(poly, (vector_t) {velocity.x, -velocity.y * body_get_elasticity(poly)});
            return;
        }
    }
}

/**
 * If the polygon_t pointed to by poly is completely passed the x coordinate of max,
 * dequeues from the poly_list_t pointed to by stars (removes the oldest, furthest right star).
 * 
 * @param stars a pointer to the poly_list_t of stars on the screen
 * @param poly a pointer to the polygon_t we want to check is still on screen
 * @param max the vector_t coordinate of the top right corner of the screen
 */
void check_offscreen(scene_t *scene, body_t *poly, vector_t max, size_t index) {
    list_t *points = body_get_shape(poly);

    for (size_t i = 0; i < list_size(points); i++) {
        if (((vector_t *) list_get(points, i))->x <= max.x) {
            return;
        }
    }

    scene_remove_body(scene, index);
}

/**
 * Updates the position of every polygon_t star in the poly_list_t pointed to by stars,
 * taking into account velocity, gravity acceleration, rotation, bounces, and going off screen.
 * 
 * @param stars a pointer to the poly_list_t of stars on the screen
 * @param accel the gravity acceleration vector to be added to each star's velocity
 * @param dt the elapsed time since the last screen draw, deciding how much the stars should have
 * translated, rotated, and accelerated
 * @param min the bottom left corner of the screen
 * @param max the top right corner of the screen
 */
void update_pos(scene_t *scene, vector_t accel, double dt, vector_t min, vector_t max) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *current_body = (body_t *) scene_get_body(scene, i);
        //list_t *points = body_get_shape(current_body);
        vector_t velocity = body_get_velocity(current_body);
        // Update velocity with dt * accel gravity vector
        body_set_velocity(current_body, vec_add(velocity, vec_multiply(dt, accel)));
        body_set_passive_rotation(current_body, ROTATION + ROTATION * dt);
        // Negate velocity if there is a bounce
        check_bounce(current_body, min);
        // Remove the "oldest" star from stars (dequeue) if a star goes offscreen
        check_offscreen(scene, current_body, max, i);
    }
    
}

/**
 * Calls sdl_draw_polygon to draw all polygons in the poly_list_t pointed to by shapes
 */
void draw_bodies(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        rgb_color_t color = body_get_color(body);
        sdl_draw_polygon(body_get_shape(body), color);
    }
}

/**
 * Adds a new star with number of STARTING_SIDES num_sides to the poly_list_t pointed to by stars
 */
void add_star(scene_t *scene, int num_sides) {
    // Decide new star properties:
    rgb_color_t color = color_init((double)rand() / (double)RAND_MAX, 
                                (double)rand() / (double)RAND_MAX, 
                                (double)rand() / (double)RAND_MAX);
    double elasticity = (double) rand() / (double) RAND_MAX * (ELAST_MAX - ELAST_MIN) + ELAST_MIN;
    vector_t entry = {ENTRY_X, ENTRY_Y};
    vector_t vel = {VEL_X, 0};

    body_t *new_star = body_init(create_star(num_sides, SIZE, entry), 1000, color);
    body_set_velocity(new_star, vel);
    body_set_elasticity(new_star, elasticity);
    body_set_rotation(new_star, ROTATION);
    body_set_passive_rotation(new_star, ROTATION);
    
    scene_add_body(scene, new_star);
}

int main(int argc, char *argv[]) {
    vector_t min = {MIN_X, MIN_Y};
    vector_t max = {MAX_X, MAX_Y};
    vector_t grav = {0, GRAV_Y};

    scene_t *scene = scene_init(); //for sdl_is_done
    double time_since_last_star = 0.0;
    int sides = STARTING_SIDES;
    
    sdl_init(min, max);
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        time_since_last_star += dt;
        
        if (time_since_last_star >= TIME_BETWEEN_STARS || sides == STARTING_SIDES) {
            // Make sure TIME_BETWEEN_STARS has elapsed since the last star unless this is the first star
            add_star(scene, sides);
            sides++;
            time_since_last_star = 0.0;
        }

        update_pos(scene, grav, dt, min, max);
        //call scene tick
        scene_tick(scene, dt);
        sdl_clear();
        draw_bodies(scene);
        sdl_show();
    }

    scene_free(scene);
}
