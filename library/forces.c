#include "vector.h"
#include "polygon.h"
#include "body.h"
#include "scene.h"
#include "forces.h"
#include "my_aux.h"
#include "collision.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

double MIN_GRAV_DISTANCE = 5.0;
double TONGUE_DRAG_CONSTANT = 200.0;


double body_distance(body_t *b1, body_t *b2){
    vector_t c1 = body_get_centroid(b1);
    vector_t c2 = body_get_centroid(b2);
    double x_dis = c2.x - c1.x;
    double y_dis = c2.y - c1.y;
    return sqrt(x_dis * x_dis + y_dis * y_dis);
}

vector_t unit_vec(body_t *b1, body_t *b2){
    vector_t c1 = body_get_centroid(b1);
    vector_t c2 = body_get_centroid(b2);
    double dis_factor = 1 / body_distance(b1, b2);
    vector_t vec = vec_multiply(dis_factor, vec_subtract(c2, c1));
    return vec;
}

void calc_grav_force(aux_t *aux){
    body_t *body1 = aux_get_body1(aux);
    body_t *body2 = aux_get_body2(aux);
    double force;
    if(body_distance(body1, body2) < MIN_GRAV_DISTANCE){
        force = 0;
    }
    else {
        force = aux_get_constant(aux) * body_get_mass(body1) * body_get_mass(body2);
        force /= pow(body_distance(body1, body2), 2);
    }
    body_add_force(body1, vec_multiply(force, unit_vec(body1, body2)));
    body_add_force(body2, vec_multiply(force, vec_multiply(-1, unit_vec(body1, body2))));

}

void calc_spring_force(aux_t *aux) {
    body_t *body1 = aux_get_body1(aux);
    body_t *body2 = aux_get_body2(aux);
    vector_t force = vec_multiply(aux_get_constant(aux), vec_subtract(body_get_centroid(body2), body_get_centroid(body1)));
    body_add_force(body1, force);
    body_add_force(body2, vec_multiply(-1, force));
}

void calc_drag_force(aux_t *aux) {
    body_t *body1 = aux_get_body1(aux);
    vector_t force = vec_multiply(aux_get_constant(aux), vec_multiply(-1, body_get_velocity(body1)));
    body_add_force(body1, force);
}

void half_destructive_collision(body_t *body1, body_t *body2, vector_t axis, void *aux){
    body_remove(body2);
}

void destructive_collision(body_t *body1, body_t *body2, vector_t axis, void *aux){
    body_remove(body1);
    body_remove(body2);
}

void calc_collision(aux_t *aux){
    body_t *body1 = aux_get_body1(aux);
    body_t *body2 = aux_get_body2(aux);
    collision_handler_t collision = (collision_handler_t) aux_get_collision(aux);
    void *aux_info = aux_get_aux_info(aux);

    if(find_collision(body_get_shape(body1), body_get_shape(body2)).collided){
        if (! aux_get_collided_last_frame(aux)) {
        collision(body1, body2, find_collision(body_get_shape(body1), body_get_shape(body2)).axis, aux_info);
        aux_set_collided_last_frame(aux, true);
        }
    }
    else if (aux_get_collided_last_frame(aux)) {
        aux_set_collided_last_frame(aux, false);
    }
}

void calc_interaction(aux_t *aux){
    body_t *body1 = aux_get_body1(aux);
    body_t *body2 = aux_get_body2(aux);
    collision_handler_t collision = (collision_handler_t) aux_get_collision(aux);
    void *aux_info = aux_get_aux_info(aux);
    collision(body1, body2, find_collision(body_get_shape(body1), body_get_shape(body2)).axis, aux_info);
}

void create_collision(
    scene_t *scene,
    body_t *body1,
    body_t *body2,
    collision_handler_t handler,
    void *aux,
    free_func_t freer
){
    aux_t *aux_info = aux_init(0, body1, body2);
    aux_set_collision(aux_info, handler);
    aux_set_aux_info(aux_info, aux);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_collision, aux_info, bodies, (free_func_t) freer);
}


void create_interaction(
    scene_t *scene,
    body_t *body1,
    body_t *body2,
    collision_handler_t handler,
    void *aux,
    free_func_t freer
){
    aux_t *aux_info = aux_init(0, body1, body2);
    aux_set_collision(aux_info, handler);
    aux_set_aux_info(aux_info, aux);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_interaction, aux_info, bodies, (free_func_t) freer);
}

void impulse_collision(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    double m1 = body_get_mass(body1);
    double m2 = body_get_mass(body2);
    assert(!(m1 == INFINITY && m2 == INFINITY) && "Tried to apply impulse to two infinite masses!");

    vector_t collision_axis = find_collision(body_get_shape(body1), body_get_shape(body2)).axis;
    double u1 = vec_dot(collision_axis, body_get_velocity(body1));
    double u2 = vec_dot(collision_axis, body_get_velocity(body2));

    double reduced_mass = m1 * m2 / (m1 + m2);
    if (m1 == INFINITY) {
        reduced_mass = m2;
    }
    else if (m2 == INFINITY) {
        reduced_mass = m1;
    }

    //aux_t *holder = (aux_t *) aux;

    // Apply formula given in project specs
    vector_t impulse = vec_multiply(reduced_mass * (1.0 + (*(double *) aux)) * (u2 - u1), collision_axis);
    //vector_t impulse = vec_multiply(reduced_mass * (1.0 + aux_get_constant(holder)) * (u2 - u1), collision_axis);

    //printf("%f %f\n", impulse.x, impulse.y);

    body_add_impulse(body1, impulse);
    body_add_impulse(body2, vec_negate(impulse));

}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1, body_t *body2) {
    aux_t *aux_info = aux_init(G, body1, body2);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_grav_force, aux_info, bodies, (free_func_t) aux_free);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
    aux_t *aux_info = aux_init(k, body1, body2);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_spring_force, aux_info, bodies, (free_func_t) aux_free);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
    aux_t *aux_info = aux_init(gamma, body, NULL);
    list_t *bodies = list_init(1, (free_func_t) body_free);
    list_add(bodies, body);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_drag_force, aux_info, bodies, (free_func_t) aux_free);
}

void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2) {
    aux_t *aux_info = aux_init(0, body1, body2);
    aux_set_aux_info(aux_info, malloc(sizeof(char)));
    aux_set_collision(aux_info, (collision_handler_t) destructive_collision);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_collision, aux_info, bodies, (free_func_t) aux_free);
}

void create_half_destruction(scene_t *scene, body_t *body1, body_t *body2) {
    aux_t *aux_info = aux_init(0, body1, body2);
    aux_set_aux_info(aux_info, malloc(sizeof(char)));
    aux_set_collision(aux_info, (collision_handler_t) half_destructive_collision);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_collision, aux_info, bodies, (free_func_t) aux_free);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1, body_t *body2) {
    aux_t *aux_info = aux_init(elasticity, body1, body2);
    double *e = malloc(sizeof(double));
    *e = elasticity;
    aux_set_aux_info(aux_info, e);
    aux_set_collision(aux_info, (collision_handler_t) impulse_collision);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_collision, aux_info, bodies, (free_func_t) aux_free);
}


void calc_tongue_force(aux_t *aux){
    body_t *body1 = aux_get_body1(aux);
    body_t *body2 = aux_get_body2(aux);

    list_t *collidables = aux_get_aux_info(aux);

    for(size_t i = 0; i < list_size(collidables); i++){
        if(find_collision(body_get_shape(body1), body_get_shape(list_get(collidables, i))).collided){
            return;
        }
        if(body_get_mass(body2) != INFINITY && find_collision(body_get_shape(body2), body_get_shape(list_get(collidables, i))).collided){
            return;
        }
    }

    vector_t force1 = vec_multiply(aux_get_constant(aux), vec_subtract(body_get_centroid(body2), body_get_centroid(body1)));
    body_add_force(body1, force1);
    body_add_force(body2, vec_multiply(-1, force1));

    vector_t force2 = vec_multiply(TONGUE_DRAG_CONSTANT, vec_multiply(-1, body_get_velocity(body1)));
    body_add_force(body1, force2);

    vector_t force3 = vec_multiply(TONGUE_DRAG_CONSTANT, vec_multiply(-1, body_get_velocity(body2)));
    body_add_force(body2, force3);
}

void calc_univ_grav_force(aux_t *aux) {
    body_t *body1 = aux_get_body1(aux);
    list_t *interactables = aux_get_aux_info(aux);
    vector_t force = {0, -1 * aux_get_constant(aux)};
    bool found = false;
    for(size_t i = 0; i < list_size(interactables); i++){
        if(find_collision(body_get_shape(body1), body_get_shape(list_get(interactables, i))).collided){
            found = true;
            //vector_t current_cen = body_get_centroid(body1);
            //body_set_centroid(body1, (vector_t) {current_cen.x, current_cen.y + 10});
            return;
        }
    }
    force = vec_multiply(body_get_mass(aux_get_body1(aux)), force);
    body_add_force(body1, force);
}

void create_tongue_force(scene_t *scene, double G, body_t *body1, body_t *body2, void *aux) {
    aux_t *aux_info = aux_init(G, body1, body2);
    // vector_t *midpoint = malloc(sizeof(vector_t));
    // *midpoint = (vector_t) vec_multiply(0.5, vec_add(body_get_centroid(body1), body_get_centroid(body2)));
    // aux_set_aux_info(aux_info, midpoint);
    aux_set_aux_info(aux_info, aux);
    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_tongue_force, aux_info, bodies, (free_func_t) aux_free);
}

void create_universal_gravity(scene_t *scene, double G, body_t *body, void* extra_info) {
    aux_t *aux_info = aux_init(G, body, NULL);
    aux_set_aux_info(aux_info, extra_info);
    list_t *bodies = list_init(1, (free_func_t) body_free);
    list_add(bodies, body);
    scene_add_bodies_force_creator(scene, (force_creator_t) calc_univ_grav_force, aux_info, bodies, (free_func_t) aux_free);
}
