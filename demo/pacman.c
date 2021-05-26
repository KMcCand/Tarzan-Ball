#include "body.h"
#include "list.h"
#include "vector.h"
#include "sdl_wrapper.h"
#include "scene.h"
#include "color.h"
#include "polygon.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

const double PACMAN_MASS = 50;
const double PACMAN_RADIUS = 50;
const double PACMAN_VELOCITY = 15.0;
const double EAT_RATIO = 0.9;
const size_t CIRC_NPOINTS = 20;
const vector_t PACMAN_START = (vector_t) {500, 250};
const rgb_color_t PACMAN_COLOR = (rgb_color_t) {1.0, 1.0, 0.0};

const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;
const double DAMPING_FACTOR = .02;
const double PACMAN_PASSIVE = .05;

const size_t PELLET_NPOINTS = 20;
const double PELLET_RADIUS = 5.0;
const rgb_color_t PELLET_COLOR = (rgb_color_t) {0.9, 0.9, 0.0};
const double TIME_BETWEEN_PELLETS = 2;

/**
 * Initializes and returns a pointer to a body_t that represents a packman.
 */
body_t *pacman_init() {
    // Represent the packman as an n-gon of CIRC_NPOINTS points
    list_t *pacman_points = list_init(CIRC_NPOINTS + 1, (free_func_t) vec_free);

    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = 5.0 / 3.0 * M_PI * i / CIRC_NPOINTS + M_PI / 6.0;
        vector_t *v = malloc(sizeof(*v));
        *v = (vector_t) {PACMAN_START.x + PACMAN_RADIUS * cos(angle), 
                         PACMAN_START.y + PACMAN_RADIUS * sin(angle)};
        list_add(pacman_points, v);
    }

    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t) {PACMAN_START.x, PACMAN_START.y};
    list_add(pacman_points, v);
    
    body_t *pac_bod = body_init(pacman_points, PACMAN_MASS, PACMAN_COLOR);
    body_redefine_centroid(pac_bod, PACMAN_START);
    return pac_bod;
}

/**
 * Checks if pacman centroid is over any of the screen borders. If so, teleports pacman
 * so that centroid is on the opposite border, preversing velocity and orientation.
 */
void check_border_teleport(scene_t *scene) {
    vector_t centroid = body_get_centroid(scene_get_body((scene_t *) scene, 0));
    double width = MAX_X - MIN_X;
    double height = MAX_Y - MIN_Y;
    vector_t teleport_vector = {0, 0};

    if (centroid.x >= MAX_X) {
        teleport_vector.x = -width;
    }
    else if (centroid.x <= MIN_X) {
        teleport_vector.x = width;
    }

    if (centroid.y >= MAX_Y) {
        teleport_vector.y = -height;
    }
    else if (centroid.y <= MIN_Y) {
        teleport_vector.y = height;
    }

    body_set_centroid(scene_get_body((scene_t *) scene, 0), vec_add(centroid, teleport_vector));
}

/**
 * Draws all of the body_t types in the scene_t pointed to by scene.
 * 
 * @param scene a pointer to a scene_t returned by scene_init
 */
void draw_bodies(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        rgb_color_t color = body_get_color(body);
        sdl_draw_polygon(body_get_shape(body), color);
    }
}

/**
 * Draws a pellet of radius PELLET_RADIUS at a random location on the screen
 * represented by scene. Adds the pellet's body_t pointer to scene so it can
 * be detected by moving pacman's mouth.
 */
void draw_random_pellet(scene_t *scene) {
    double rand_x = (double) rand() / (double) RAND_MAX * (MAX_X - MIN_X) + MIN_X;
    double rand_y = (double) rand() / (double) RAND_MAX * (MAX_Y - MIN_Y) + MIN_Y;

    list_t *pellet_points = list_init(PELLET_NPOINTS, (free_func_t) vec_free);

    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = i * 2.0 * M_PI / CIRC_NPOINTS;

        vector_t *v = malloc(sizeof(*v));
        *v = (vector_t) {rand_x + PELLET_RADIUS * cos(angle), 
                    rand_y + PELLET_RADIUS * sin(angle)};
        list_add(pellet_points, v);
    }
    
    scene_add_body(scene, body_init(pellet_points, INFINITY, PELLET_COLOR));
}

/**
 * Initializes and returns a pointer to a body_t that represents a packman.
 */
void check_eaten_pellets(scene_t *scene){
    // will check if the pellet is within some radius of the centroid
    // of pacman. This is cleanest implementation since pac man only moves
    // forward and so anything that gets inside some radius will have passed
    // through the mouth.
    double eat_rad = EAT_RATIO * (PACMAN_RADIUS + PELLET_RADIUS);
    vector_t pac_center = body_get_centroid(scene_get_body((scene_t *) scene, 0));
    // Assume pacman is the first body in scene and check all other bodies
    for (int i = 1; i < scene_bodies(scene); i++){
        vector_t pellet_center = body_get_centroid(scene_get_body(scene, i));
        vector_t to_center = vec_subtract(pellet_center, pac_center);
        double center_dist = sqrt(vec_dot(to_center, to_center));
        if (center_dist < eat_rad) {
            scene_remove_body(scene, i);
        }
    }
}

void draw_starting_pellets(scene_t *scene) {
    for (int i = 0; i < 20; i++){
        draw_random_pellet(scene);
    }
}


void on_key(char key, key_event_type_t type, double held_time, void *scene) {
    body_t *PACMAN = scene_get_body((scene_t *) scene, 0);
    if (type == KEY_PRESSED) {
        switch (key) {
            case UP_ARROW:
                body_set_velocity(PACMAN, vec_multiply(held_time / DAMPING_FACTOR, (vector_t) {0, PACMAN_VELOCITY}));
                body_set_rotation(PACMAN, M_PI / 2);
                break;
            case DOWN_ARROW:
                body_set_velocity(PACMAN, vec_multiply(held_time / DAMPING_FACTOR, (vector_t) {0, -1 * PACMAN_VELOCITY}));
                body_set_rotation(PACMAN, 3 * M_PI / 2);
                break;
            case LEFT_ARROW:
                body_set_velocity(PACMAN, vec_multiply(held_time / DAMPING_FACTOR, (vector_t) {-1 * PACMAN_VELOCITY, 0}));
                body_set_rotation(PACMAN, M_PI);
                break;
            case RIGHT_ARROW:
                body_set_velocity(PACMAN, vec_multiply(held_time / DAMPING_FACTOR, (vector_t) {PACMAN_VELOCITY, 0}));
                body_set_rotation(PACMAN, 0);
                break;
        }
    }
    if(type == KEY_RELEASED){
        switch (key) {
            case UP_ARROW:
                body_set_velocity(PACMAN, vec_multiply(PACMAN_PASSIVE / DAMPING_FACTOR, (vector_t) {0, PACMAN_VELOCITY}));
                break;
            case DOWN_ARROW:
                body_set_velocity(PACMAN, vec_multiply(PACMAN_PASSIVE / DAMPING_FACTOR, (vector_t) {0, -1 * PACMAN_VELOCITY}));
                break;
            case LEFT_ARROW:
                body_set_velocity(PACMAN, vec_multiply(PACMAN_PASSIVE / DAMPING_FACTOR, (vector_t) {-1 * PACMAN_VELOCITY, 0}));
                break;
            case RIGHT_ARROW:
                body_set_velocity(PACMAN, vec_multiply(PACMAN_PASSIVE / DAMPING_FACTOR, (vector_t) {PACMAN_VELOCITY, 0}));
                break;
        }
    } 
}

int main (int argc, char *argv[]) {
    // Initialize SDL
    vector_t min = {MIN_X, MIN_Y};
    vector_t max = {MAX_X, MAX_Y};
    sdl_init(min, max);

    scene_t *scene = scene_init();
    scene_add_body(scene, pacman_init());
    body_set_velocity(scene_get_body((scene_t *) scene, 0), (vector_t) {0, 0});
    
    draw_starting_pellets(scene);
    double dt = 0;
    double time_since_last_pellet = 0;

    while (!sdl_is_done(scene)) {
        dt = time_since_last_tick();
        time_since_last_pellet += dt;

        if (time_since_last_pellet >= TIME_BETWEEN_PELLETS) {
            draw_random_pellet(scene);
            time_since_last_pellet = 0;
        }

        sdl_on_key(on_key);
        check_eaten_pellets(scene);
        check_border_teleport(scene);
        scene_tick(scene, dt);
        
        sdl_render_scene(scene);
    }

    scene_free(scene);
}
