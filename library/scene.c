#include "list.h"
#include "body.h"
#include "my_aux.h"
#include "scene.h"
#include "image.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct force {
    force_creator_t forcer;
    list_t *bodies;
    aux_t *aux;
    free_func_t free_func;
}force_t;

typedef struct scene {
    list_t *bodies;
    list_t *forces;
    size_t size;
    bool has_background;
    image_t *background;
} scene_t;

force_t *force_init(force_creator_t forcer, list_t *bodies, aux_t *aux, free_func_t freer) {
    force_t *force = malloc(sizeof(force_t));
    force->forcer = forcer;
    force->bodies = bodies;
    force->aux = aux;
    force->free_func = freer;
    return force;
}

list_t *force_get_bodies(force_t *force){
    return force->bodies;
}

void *force_get_forcer(force_t *force){
    return force->forcer;
}

void force_free(force_t *force){
    if(force->free_func != NULL){
        force->free_func(force->aux);
    }
    free(force);
}

scene_t *scene_init(void) {
    scene_t *scene = malloc(sizeof(scene_t));
    scene->bodies = list_init(10, (free_func_t) body_free);
    scene->forces = list_init(10, (free_func_t) force_free);
    scene->size = 0;
    assert(scene != NULL);
    scene->background = NULL;
    scene->has_background = false;
    return scene;
}

void scene_free(scene_t *scene) {
    list_free(scene->bodies);
    list_free(scene->forces);
    if (scene->has_background) {
        image_free(scene->background);
    }
    free(scene);
}

size_t scene_bodies(scene_t *scene) {
    return scene->size;
}

body_t *scene_get_body(scene_t *scene, size_t index) {
    return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
    list_add(scene->bodies, body);
    scene->size++;
}

void scene_remove_body(scene_t *scene, size_t index) {
    body_remove(scene_get_body(scene, index));
}

void scene_set_background(scene_t *scene, char *name, vector_t dimensions) {
    scene->background = image_init(name, dimensions, 0);
}

image_t *scene_get_background(scene_t *scene) {
    assert(scene->background != NULL && "The scene does not have a background!");
    return scene->background;
}

void scene_remove_body_extra(scene_t *scene, size_t index){
    body_t *removed = list_remove(scene->bodies, index);
    body_free(removed);
    scene->size--;
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux, free_func_t freer) {
    // aux is all info passed into force creator
    // populate new struct here w/ force aux etc.
    list_t *nothing = list_init(0, (free_func_t) body_free);
    force_t *force = force_init(forcer, nothing, aux, freer);
    list_add(scene->forces, force);
}

void scene_add_bodies_force_creator(
    scene_t *scene,
    force_creator_t forcer,
    void *aux,
    list_t *bodies,
    free_func_t freer
){
    force_t *force = force_init(forcer, bodies, aux, freer);
    list_add(scene->forces, force);
}

list_t *scene_get_forces(scene_t *scene){
    return scene->forces;
}

void scene_tick(scene_t *scene, double dt){
    list_t *forces = scene->forces;

    //tick non collisions
    // for(size_t i = 0; i < list_size(forces); i++){
    //     force_t *force = list_get(forces, i);
    //     aux_t *aux = force->aux;
    
    // }

    for (size_t i = 0; i < list_size(forces); i++) {
        force_t *force = list_get(forces, i);
        aux_t *aux = force->aux;
        force->forcer(aux);
    }

    for (size_t i = 0; i < list_size(forces); i++) {
        force_t *force = list_get(forces, i);
        for(size_t j = 0; j < list_size(force->bodies); j++){
            if(body_is_removed(list_get(force->bodies, j))){
                force_t *removed = list_remove(scene->forces, i);
                i--;
                force_free(removed);
                break;
            }
        }
    }
    
    for(size_t i = 0; i < scene->size; i++){
        body_tick(list_get(scene->bodies, i), dt);
        if(body_is_removed(list_get(scene->bodies, i))){
            scene_remove_body_extra(scene, i);
            i--;
        }
    }
}
