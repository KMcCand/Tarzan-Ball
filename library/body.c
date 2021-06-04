#include "body.h"
#include "polygon.h"
#include "image.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

const size_t INITIAL_COLLIDING_BODIES_LENGTH = 3;
const double IMG_CHANGE_TIME = 0.03;

typedef struct body {
    list_t *points;
    vector_t velocity;
    vector_t centroid;
    rgb_color_t color;
    vector_t force;
    vector_t impulse;
    free_func_t info_free;
    void *info;
    double mass;
    double passive_rotation;
    double rotation;
    double elasticity;
    bool removed;
    list_t *image_list;
    bool has_image_list;
    double image_change_count;
    size_t image_list_index;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL && "Could not allocate memory for new body!");
    body->info = malloc(sizeof(void *));

    body->points = shape;
    body->velocity = (vector_t) {0, 0};
    body->centroid = polygon_centroid(shape);
    body->color = color;
    body->force = (vector_t) {0,0};
    body->impulse = (vector_t) {0,0};
    body->mass = mass;
    body->passive_rotation = 0.0;
    body->rotation = 0.0;
    body->elasticity = 1.0;
    body->info = NULL;
    body->info_free = NULL;
    body->removed = false;
    body->image_list = NULL;
    body->has_image_list = false;
    body->image_change_count = 0;
    body->image_list_index = 0;
    return body;
}

body_t *body_init_with_info(
    list_t *shape,
    double mass,
    rgb_color_t color,
    void *info,
    free_func_t info_freer
){
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL && "Could not allocate memory for new body!");
    assert(mass > 0);
    body->info = malloc(sizeof(void *));

    body->points = shape;
    body->velocity = (vector_t) {0, 0};
    body->centroid = polygon_centroid(shape);
    body->color = color;
    body->force = (vector_t) {0,0};
    body->impulse = (vector_t) {0,0};
    body->mass = mass;
    body->passive_rotation = 0.0;
    body->rotation = 0.0;
    body->elasticity = 1.0;
    body->info = info;
    body->info_free = info_freer;
    body->removed = false;
    body->image_list = NULL;
    body->has_image_list = false;
    body->image_change_count = 0;
    body->image_list_index = 0;
    return body;
}

void body_free(body_t *body){
    if (body->info_free != NULL) {
         body->info_free(body->info);
    }
    list_free(body->points);
    if (body->has_image_list) {
        list_free(body->image_list);
    }
    free(body);
}

void body_add_image_list(body_t *body, list_t *image_list) {
    if (list_size(image_list) > 0) {
        body->image_list = image_list;
        body->has_image_list = true;
    }
}

bool body_has_image_list(body_t *body) {
    return body->has_image_list;
}

image_t *body_get_current_image(body_t *body) {
    return list_get(body->image_list, body->image_list_index);
}

vector_t *give_vec(void) {
    vector_t *holder = malloc(sizeof(double) * 2);
    return holder;
}

list_t *body_get_shape(body_t *body) {
    list_t *points_copy = list_init(list_size(body->points), (free_func_t) vec_free);
    for (int i = 0; i < list_size(body->points); i++) {
        vector_t *thing = give_vec();
        memcpy(thing, list_get(body->points, i), sizeof(vector_t));
        list_add(points_copy, thing);
    }
    return points_copy;
}

vector_t body_get_centroid(body_t *body) {
    return body->centroid;
}

double body_get_elasticity(body_t *body) {
    return body->elasticity;
}

double body_get_rotation(body_t *body) {
    return body->rotation;
}

vector_t body_get_velocity(body_t *body) {
    return body->velocity;
}

double body_get_mass(body_t *body) {
    return body->mass;
}

rgb_color_t body_get_color(body_t *body) {
    return body->color;
}

void body_set_elasticity(body_t *body, double new_elasticity) {
    body->elasticity = new_elasticity;
}

void body_set_centroid(body_t *body, vector_t x) {
    polygon_translate(body->points, vec_add(x, vec_negate(body->centroid)));
    body->centroid = x;
}

void body_redefine_centroid(body_t *body, vector_t new_centroid) {
    body->centroid = new_centroid;
}

void body_set_velocity(body_t *body, vector_t v) {
    body->velocity = v;
}

void body_set_passive_rotation(body_t *body, double new_passive_rotation) {
    body->passive_rotation = new_passive_rotation;
}

void body_set_rotation(body_t *body, double angle) {
    polygon_rotate(body->points, angle - body->rotation, body->centroid);
    body->rotation = angle;
}

void *body_get_info(body_t *body) {
    assert(body->info != NULL);
    return body->info;
}

void body_remove(body_t *body) {
    body->removed = true;
}

bool body_is_removed(body_t *body) {
    return body->removed;
}

void body_tick(body_t *body, double dt) {
    vector_t new_vel = vec_add(body->velocity, vec_multiply(dt / body->mass, body->force));
    new_vel = vec_add(new_vel, vec_multiply(1/body->mass, body->impulse));

    vector_t avg_vel = vec_multiply(0.5, vec_add(new_vel, body->velocity));

    double new_x = body->centroid.x + avg_vel.x * dt;
    double new_y = body->centroid.y + avg_vel.y * dt;
    body_set_centroid(body, (vector_t) {new_x, new_y});
    body_set_rotation(body, body->rotation + body->passive_rotation * dt);
    
    body->velocity = new_vel;
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
    
    if (body->has_image_list) {
        body->image_change_count += dt;

        if (body->image_change_count > IMG_CHANGE_TIME) {
            body->image_list_index = (body->image_list_index + 1) % list_size(body->image_list);
            body->image_change_count = 0;
        }
    }
}

void body_add_force(body_t *body, vector_t force) {
    body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse){
    body->impulse = vec_add(body->impulse, impulse);
}
