#include "body.h"
#include "collision.h"
#include <float.h>
#include "list.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "scene.h"
#include "forces.h"

/**
 * Determines the minimum vector projection of the points of
 * a polygon onto an axis.
 *
 * @param shape the shape
 * @param vec the axis being projected onto
 * @return the minimum value of a point projected onto the axis
 */
double min_proj(list_t *shape, vector_t *vec) {
    double min_val = 10000000;
    for(int i = 0; i < list_size(shape); i++) {
        double val = vec_dot(*(vector_t*) list_get(shape, i), *vec);
        if(val < min_val) {
            min_val = val;
        }
    }
    return min_val;
}

/**
 * Determines the maximum vector projection of the points of
 * a polygon onto an axis.
 *
 * @param shape the shape
 * @param vec the axis being projected onto
 * @return the maximum value of a point projected onto the axis
 */
double max_proj(list_t *shape, vector_t *vec) {
    double max_val = -10000000;
    for(int i = 0; i < list_size(shape); i++) {
        double val = vec_dot(*(vector_t*) list_get(shape, i), *vec);
        if(val > max_val) {
            max_val = val;
        }
    }
    return max_val;
}

/**
 * Determines if two ranges are overlapping
 *
 * @param min1 the minimum of range1
 * @param max1 the maximum of range1
 * @param min2 the minimum of range2
 * @param max2 the maximum of range2
 * @return if the two ranges are overlapping
 */
double overlap(double min1, double max1, double min2, double max2) {
    if((min2 > min1 && min2 < max1))
        return max1 - min2;
    else if (max2 > min1 && max2 < max1) 
        return max2 - min1;
    else if (min2 < min1 && max2 > max1)
        return max1 - min1;
    return 0;
}


collision_info_t find_collision(list_t *shape1, list_t *shape2) {
    size_t size1 = list_size(shape1);
    size_t size2 = list_size(shape2);
    list_t *axes = list_init(size1 + size2, (free_func_t) vec_free);
    for(size_t i = 0; i < size1; i++) {
        vector_t *vec1 = list_get(shape1, i % size1);
        vector_t *vec2 = list_get(shape1, (i + 1) % size1);
        vector_t edge = vec_subtract(*vec2, *vec1);
        vector_t *vec_perp = malloc(sizeof(vector_t));
        *vec_perp = (vector_t) {edge.y, -1 * edge.x};
        list_add(axes, vec_perp);
    }
    for(size_t i = 0; i < size2; i++) {
        vector_t *vec1 = list_get(shape2, i % size2);
        vector_t *vec2 = list_get(shape2, (i + 1) % size2);
        vector_t edge = vec_subtract(*vec2, *vec1);
        vector_t *vec_perp = malloc(sizeof(vector_t));
        *vec_perp = (vector_t) {edge.y, -1 * edge.x};
        list_add(axes, vec_perp);
    }
    double min_overlap = 10000000;
    collision_info_t ret = {false, VEC_ZERO};
    for(size_t i = 0; i < list_size(axes); i++) {
        vector_t *axis = list_get(axes, i);
        double min1 = min_proj(shape1, axis);
        double max1 = max_proj(shape1, axis);
        double min2 = min_proj(shape2, axis);
        double max2 = max_proj(shape2, axis);
        double curr = overlap(min1, max1, min2, max2);
        if(curr < min_overlap) {
            if (curr == 0.0) {
                ret.collided = false;
                ret.axis = vec_multiply(1/sqrt(pow((*axis).x, 2) + pow((*axis).y, 2)), *axis);
                return ret;
            }
            else {
                min_overlap = curr;
                ret.axis = vec_multiply(1/sqrt(pow((*axis).x, 2) + pow((*axis).y, 2)), *axis);
            }
        }
    }
    ret.collided = true;
    list_free(axes);
    return ret;
}
