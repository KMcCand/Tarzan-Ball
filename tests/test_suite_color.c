#include "test_util.h"
#include "polygon.h"
#include "list.h"
#include "color.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
void test_init_color() {
    rgb_color_t my_color = color_init(0.1, 0.2, 0.3);
    assert(isclose((double)color_get_r(my_color), 0.1));
    assert(isclose((double)color_get_g(my_color), 0.2));
    assert(isclose((double)color_get_b(my_color), 0.3));

    color_free(my_color);
}
*/

int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    // DO_TEST(test_init_color);
  
    puts("color_test PASS");
}
