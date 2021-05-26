#include "vector.h"
#include "list.h"
#include "color.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Computes the summations of the shoelace formula
 * 
 * @param polygon the list of vertices that make up the polygon
 * @param i the index of the point in the list
 * @return the summation for the given set of points
 */
double shoelace(list_t *polygon, size_t i) {
    // x_i * y_{i + 1} - x_{i + 1} * y_i
    return (((vector_t *) list_get(polygon, i))->x 
          * ((vector_t *) list_get(polygon, (i + 1) % list_size(polygon)))->y) 
         - (((vector_t *) list_get(polygon, (i + 1) % list_size(polygon)))->x
            * ((vector_t *) list_get(polygon, i))->y);

}

double polygon_area(list_t *polygon) {
    double ret = 0;
    for(size_t i = 0; i < list_size(polygon); i++) {
        ret += shoelace(polygon, i);
    }
    return 0.5 * fabs(ret);
}

vector_t polygon_centroid(list_t *polygon) {
    // Using the centroid formula here: https://en.wikipedia.org/wiki/Centroid#Of_a_polygon
    double centroid_x = 0.0;
    double centroid_y = 0.0;
    size_t size = list_size(polygon);

    for (size_t i = 0; i < size; i++) {
        // We are using modulus to cause the last i to wrap around to 0
        vector_t *current = (vector_t *) list_get(polygon, i % size);
        vector_t *next = (vector_t *) list_get(polygon, (i + 1) % size);

        double second_part = vec_cross(*current, *next);
        centroid_x += (current->x + next->x) * second_part;
        centroid_y += (current->y + next->y) * second_part;
    }
    
    double divisor = 6.0 * polygon_area(polygon);
    vector_t answer = {fabs(centroid_x / divisor), fabs(centroid_y / divisor)};
    return answer;
}

void polygon_translate(list_t *polygon, vector_t translation) {
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *vec = (vector_t *) list_get(polygon, i);
        *vec = vec_add(*vec, translation);
    }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
    for (size_t i = 0; i < list_size(polygon); i++) {
            // translate each point to the origin, rotate it around, 
            // then translate it back against the original translation
            vector_t *vec = (vector_t *) list_get(polygon, i);
            *vec = vec_subtract(*vec, point);
            *vec = vec_rotate(*vec, angle);
            *vec = vec_add(*vec, point);
    }
}
