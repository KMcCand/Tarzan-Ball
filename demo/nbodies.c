#include "forces.h"
#include "scene.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "my_aux.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

const size_t N_BODIES = 40;
const size_t N_BODY_SIDES = 4;
const double RAND_SEED = 50; //if want to test on different "scenario"

const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;

const double MIN_MASS = 50;
const double MAX_MASS = 200;
const double MIN_SIZE = 15;
const double MAX_SIZE = 40;

const double GRAVITY = .4;

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

void add_star(scene_t *scene, int num_sides) {
    // Decide new star properties:
    rgb_color_t color = color_init((double)rand() / (double)RAND_MAX, 
                                (double)rand() / (double)RAND_MAX, 
                                (double)rand() / (double)RAND_MAX);

    double x_coord = fmod((double)rand(), MAX_X - MIN_X + 1) + MIN_X;
    double y_coord = fmod((double)rand(), MAX_Y - MIN_Y + 1) + MIN_Y;
    vector_t center = (vector_t) {x_coord, y_coord};

    double mass = fmod((double)rand(), MAX_MASS - MIN_MASS + 1) + MIN_MASS;
    double size = fmod((double)rand(), MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE;

    body_t *new_star = body_init(create_star(num_sides, size, center), mass, color);

    scene_add_body(scene, new_star);
}

void generate_start_bodies(scene_t *scene){
    for(size_t i = 0; i < N_BODIES; i++){
        add_star(scene, N_BODY_SIDES);
    }   
}

void create_gravity(scene_t *scene){
    for (size_t i = 0; i < scene_bodies(scene); i++){
        for (size_t j = i; j < scene_bodies(scene); j++){
            if (i != j){
                create_newtonian_gravity(scene, GRAVITY, scene_get_body(scene, i), scene_get_body(scene, j));
            }
        }
    }
}

int main(int argc, char *argv[]){
    vector_t min = {MIN_X, MIN_Y};
    vector_t max = {MAX_X, MAX_Y};
    srand(RAND_SEED);

    scene_t *scene = scene_init();
    sdl_init(min, max);
    generate_start_bodies(scene);

    while(!sdl_is_done(scene)){
        double dt = time_since_last_tick();
        create_gravity(scene);
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }

    scene_free(scene);
}
