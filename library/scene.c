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
    list_t *text_images;
    bool pause;
    bool clicked;
    void *extra_info;
    free_func_t extra_info_freer;
} scene_t;

force_t *force_init(force_creator_t forcer, list_t *bodies, aux_t *aux, free_func_t freer) {
    force_t *force = malloc(sizeof(force_t));
    force->forcer = forcer;
    force->bodies = bodies;
    force->aux = aux;
    force->free_func = freer;
    return force;
}

list_t *force_get_bodies(force_t *force) {
    return force->bodies;
}

void *force_get_forcer(force_t *force) {
    return force->forcer;
}

bool scene_has_background(scene_t *scene) {
    return scene->has_background;
}

void force_free(force_t *force) {
    if(force->free_func != NULL){
        force->free_func(force->aux);
    }
    free(force);
}

scene_t *scene_init(void) {
    scene_t *scene = malloc(sizeof(scene_t));
    scene->bodies = list_init(10, (free_func_t) body_free);
    scene->forces = list_init(10, (free_func_t) force_free);
    scene->extra_info = malloc(sizeof(void *));
    scene->size = 0;
    assert(scene != NULL);
    scene->background = NULL;
    scene->has_background = false;
    scene->text_images = list_init(3, (free_func_t) image_free);
    scene->pause = false;
    scene->clicked = false;
    scene->extra_info_freer = NULL;
    return scene;
}

void scene_free(scene_t *scene) {
    list_free(scene->bodies);
    list_free(scene->forces);
    if (scene->has_background) {
        image_free(scene->background);
    }
    list_free(scene->text_images);
    if(scene->extra_info_freer != NULL){
        scene->extra_info_freer(scene->extra_info);
    }
    free(scene);
}

size_t scene_bodies(scene_t *scene) {
    return scene->size;
}

body_t *scene_get_body(scene_t *scene, size_t index) {
    return list_get(scene->bodies, index);
}

void *scene_get_extra_info(scene_t *scene){
    return scene->extra_info;
}

void scene_set_extra_info(scene_t *scene, void *info, free_func_t freer){
    scene->extra_info = info;
    scene->extra_info_freer = freer;
}

void scene_add_body(scene_t *scene, body_t *body) {
    list_add(scene->bodies, body);
    scene->size++;
}

void scene_remove_body(scene_t *scene, size_t index) {
    body_remove(scene_get_body(scene, index));
}

void scene_set_background(scene_t *scene, char *name, vector_t dimensions) {
    scene->has_background = true;
    scene->background = image_init(name, dimensions, 0);
}

image_t *scene_get_background(scene_t *scene) {
    assert(scene->background != NULL && "The scene does not have a background!");
    return scene->background;
}

void scene_add_text_image(scene_t *scene, char *name, vector_t dimensions) {
    image_t *new_image = image_init(name, dimensions, 0);
    image_set_show(new_image, false);
    list_add(scene->text_images, new_image);
}

list_t *scene_get_text_images(scene_t *scene) {
    return scene->text_images;
}

bool scene_get_clicked(scene_t *scene){
    return scene->clicked;
}

void scene_set_clicked(scene_t *scene, bool clicked){
    scene->clicked = clicked;
}

void scene_set_show_text_image(scene_t *scene, size_t index, bool new_value) {
    image_set_show(list_get(scene->text_images, index), new_value);
}

bool scene_show_text_image(scene_t *scene, size_t index) {
    return image_get_show(list_get(scene->text_images, index));
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

void scene_set_pause(scene_t *scene, bool value) {
    scene->pause = value;
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

void scene_tick(scene_t *scene, double dt) {
    if (! scene->pause) { 
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
    else {
        for(size_t i = 0; i < scene->size; i++) {
            body_t *current_body = list_get(scene->bodies, i);
            if (((char *) body_get_info(current_body))[0] == 'C' || ((char *) body_get_info(current_body))[0] == 'I') {
                body_tick(current_body, dt);
            }
        }
    }   
}
