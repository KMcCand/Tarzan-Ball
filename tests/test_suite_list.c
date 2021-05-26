// #include "test_util.h"
// #include "polygon.h"
// #include "list.h"
// #include "color.h"
// #include "body.h"
// #include <assert.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <math.h>


// list_t *star_points(int n, int size, vector_t center) {
//     list_t *ret = list_init(2 * n, (free_func_t)vec_free);
//     // golden ratio between inner and outer radii of stars
//     double ratio = (3 + sqrt(5))/2;
//     for(int i = 0; i < 2 * n; i++) {
//         vector_t *vec = malloc(sizeof(vector_t));
//         if(i % 2 == 0) {
//             *vec = (vector_t) {.x = center.x + size * cos(M_PI * i / n + M_PI / 2), 
//                                .y = center.y + size * sin(M_PI * i / n + M_PI / 2)};
//         }
//         else {
//             *vec = (vector_t) {.x = center.x + size/ratio * cos(M_PI * i / n + M_PI / 2), 
//                                .y = center.y + size/ratio * sin(M_PI * i / n + M_PI / 2)};
//         }
//         list_add_front(ret, vec);
//     }
//     return ret;
// }

// void test_list_size0() {
//     list_t *l = list_init(0, (free_func_t)vec_free);
//     assert(list_size(l) == 0);
//     list_free(l);
// }

// void test_list_size1() {
//     list_t *l = list_init(1, (free_func_t)body_free);
//     assert(list_size(l) == 0);

//     vector_t vel = (vector_t) {10, 10};
//     rgb_color_t *color = rgb_color_init(0, 0, 0);
//     body_t *v = polygon_init(star_points(5, 30, (vector_t) {0,0}), 100, color);
//     list_add_front(l, v);
//     assert(list_size(l) == 1);
//     assert(color_get_r(body_get_color((body_t *)list_get(l, 0))) == 0); //check color works
//     assert(list_remove(l, 0) == v);
//     assert(list_size(l) == 0);
//     list_add_front(l, v);

//     //edit the body inside the list
//     body_set_rotation(list_get(l, 0), 0.1);    
//     body_set_velocity(list_get(l, 0), (vector_t) {15, 15});
//     assert(vec_isclose(body_get_velocity(list_get(l, 0)), (vector_t) {15, 15}));

//     assert(isclose(body_get_rotation(v), 20));

//     list_free(l);
// }

// void size_overload(){
//     list_t *l = list_init(1, (free_func_t)body_free);

//     body_t *p = body_init(star_points(5, 30, (vector_t) {0,0}), 100, color_init(0.1, 0.1, 0.1));
//     body_t *p2 = body_init(star_points(6, 30, (vector_t) {0,0}), 100, color_init(0.2,  0.2, 0.2));
//     body_t *p3 = body_init(star_points(7, 30, (vector_t) {0,0}), 100, color_init(0.3, 0.3, 0.3));

//     list_add_front(l, p);
//     list_add_front(l, p2);
//     list_add_front(l, p3);

//     assert(list_size(l) == 3);
//     assert(list_capacity(l) == 4);

//     assert(isclose((double)color_get_r(body_get_color(list_remove(l, 0))), .1));
//     assert(isclose((double)color_get_g(body_get_color(list_remove(l, 0))), .2));
//     assert(isclose((double)color_get_b(body_get_color(list_remove(l, 0))), .3));

//     list_free(l);
// }

int main(int argc, char *argv[]) {
    // // Run all tests if there are no command-line arguments
    // bool all_tests = argc == 1;
    // // Read test name from file
    // char testname[100];
    // if (!all_tests) {
    //     read_testname(argv[1], testname, sizeof(testname));
    // }

    // DO_TEST(test_list_size0);
    // DO_TEST(test_list_size1);
    // DO_TEST(size_overload);
  
    //puts("list_test PASS");
}
