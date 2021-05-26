#include "../include/vector.h"
#include <stdlib.h>
#include <math.h>

const vector_t VEC_ZERO = {0, 0};

void vec_free(vector_t *v){
    free(v);
}

vector_t vec_add(vector_t v1, vector_t v2) {
    vector_t ret = {.x = v1.x + v2.x, .y = v1.y + v2.y};
    return ret;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
    vector_t ret = vec_add(v1, vec_negate(v2));
    return ret;
}

vector_t vec_negate(vector_t v) {
    vector_t ret = vec_multiply(-1, v);
    return ret;
}

vector_t vec_multiply(double scalar, vector_t v) {
    vector_t ret = {.x = v.x * scalar, .y = v.y * scalar};
    return ret;
}

double vec_dot(vector_t v1, vector_t v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double vec_cross(vector_t v1, vector_t v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

vector_t vec_rotate(vector_t v, double angle) {
    vector_t ret = {v.x * cos(angle) - v.y * sin(angle), v.x * sin(angle) + v.y * cos(angle)};
    return ret;
}
