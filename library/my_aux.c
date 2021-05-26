#include "body.h"
#include "forces.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct aux {
    double constant;
    bool collided_last_frame;
    body_t *body1;
    body_t *body2;
    void *aux_info;
    void *collision;
} aux_t;

aux_t *aux_init(double constant, body_t *body1, body_t *body2) {
    aux_t *new_aux = malloc(sizeof(aux_t));
    new_aux->body1 = body1;
    if (body2 != NULL) {
        new_aux->body2 = body2;
    }
    new_aux->constant = constant;
    assert(new_aux != NULL);

    new_aux->aux_info = malloc(sizeof(void *));
    new_aux->aux_info = NULL;
    new_aux->collision = NULL;
    new_aux->collided_last_frame = false;

    return new_aux;
}

double aux_get_constant(aux_t *aux) {
    return aux->constant;
}

bool aux_get_collided_last_frame(aux_t *aux) {
    return aux->collided_last_frame;
}

body_t *aux_get_body1(aux_t *aux) {
    return aux->body1;
}

body_t *aux_get_body2(aux_t *aux) {
    return aux->body2;
}

void *aux_get_collision(aux_t *aux) {
    assert(aux->collision != NULL);
    return aux->collision;
}

void *aux_get_aux_info(aux_t *aux) {
    assert(aux->aux_info != NULL);
    return aux->aux_info;
}

void aux_free(aux_t *aux) {
    free(aux);
}

void aux_set_collision(aux_t *aux, void *collision) {
    aux->collision = collision;
}

void aux_set_aux_info(aux_t *aux, void *info){
    aux->aux_info = info;
}

void aux_set_collided_last_frame(aux_t *aux, bool new_val) {
    aux->collided_last_frame = new_val;
}
