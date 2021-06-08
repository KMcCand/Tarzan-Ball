#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "collision.h"

const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;

body_t *rect_gen(scene_t *scene, double width, double height, double mass, vector_t center, rgb_color_t color, char c, double rotate){
    list_t *rect_pts = list_init(4, (free_func_t) vec_free);

    vector_t *v = malloc(sizeof(*v));
    *v = vec_add(center, (vector_t) {.5 * width, .5 * height});
    list_add(rect_pts, v);
    v = malloc(sizeof(*v));
    *v = vec_add(center, (vector_t) {.5 * width, -.5 * height});
    list_add(rect_pts, v);
    v = malloc(sizeof(*v));
    *v = vec_add(center, (vector_t) {-.5 * width, -.5 * height});
    list_add(rect_pts, v);
    v = malloc(sizeof(*v));
    *v = vec_add(center, (vector_t) {-.5 * width, .5 * height});
    list_add(rect_pts, v);

    polygon_rotate(rect_pts, rotate, center);

    char *test = malloc(1);
    *test = c;

    body_t *bod = body_init_with_info(rect_pts, mass, color, test, NULL);
    scene_add_body(scene, bod);
    return bod;
}

body_t *circle_g(scene_t *scene, size_t points, vector_t start, double radius, double mass, rgb_color_t color, char c){
    list_t *ball_points = list_init(points, (free_func_t) vec_free);
    for (size_t i = 0; i < points; i++) {
        double angle = 2 * M_PI * i / points; 
        vector_t *v = malloc(sizeof(*v));
        *v = (vector_t) {start.x + radius * cos(angle), 
                         start.y + radius * sin(angle)};
        list_add(ball_points, v);
    }
    polygon_rotate(ball_points, M_PI / points, start);
    char *test = malloc(1);
    *test = c;
    body_t *ball_bod = body_init_with_info(ball_points, mass, color, test, (free_func_t) free);
    scene_add_body(scene, ball_bod);
    return ball_bod;
}

void circ_draw(scene_t *scene, char *line){
    list_t *list = list_init(10, free);

    char *ptr = malloc(200);
	const char delim[2] = " ";

    char *holder = malloc(100);

	ptr = strtok(line, delim);

	while(ptr != NULL){
		list_add(list, ptr);
		ptr = strtok(NULL, delim);
	}

    char *c = malloc(1);
    if(strtod(list_get(list, 8), &holder) == 0){
        printf("palyer");
        *c = 'P';
    }
    else{
        printf("target");
        *c = 'E';
    }

    circle_g(scene, strtod(list_get(list, 0), &holder), (vector_t) {strtod(list_get(list, 1), &holder), strtod(list_get(list, 2), &holder)}, strtod(list_get(list, 3), &holder), strtod(list_get(list, 4), &holder), (rgb_color_t) {strtod(list_get(list, 5), &holder), strtod(list_get(list, 6), &holder), strtod(list_get(list, 7), &holder)}, *c);
}

void rect_draw(scene_t *scene, char *line){
    //pulled from: https://www.codingame.com/playgrounds/14213/how-to-play-with-strings-in-c/string-split
    list_t *list = list_init(10, free);

    char *ptr = malloc(200);
	const char delim[2] = " ";

    char *holder = malloc(100);

	ptr = strtok(line, delim);

	while(ptr != NULL){
		list_add(list, ptr);
		ptr = strtok(NULL, delim);
	}
    rect_gen(scene, strtod(list_get(list, 0), &holder), strtod(list_get(list, 1), &holder), strtod(list_get(list, 2), &holder), (vector_t) {strtod(list_get(list, 3), &holder), strtod(list_get(list, 4), &holder)}, (rgb_color_t) {strtod(list_get(list, 5), &holder), strtod(list_get(list, 6), &holder), strtod(list_get(list, 7), &holder)}, 'W', strtod(list_get(list, 9), &holder));
}

int main(int argc, char *argv[]) {
    FILE *f = fopen("levels/level_TESTING.txt", "r");

    sdl_init((vector_t) {MIN_X, MIN_Y}, (vector_t) {MAX_X, MAX_Y});
    scene_t *scene = scene_init();

    char *line = malloc(200);

    int part = 0;

    while(fgets(line, 200, f)){
        //printf("%s", line);
        //rect_draw(scene, line);
        if(strcmp(line, "WALLS\n") == 0){
            part = 1;
        }
        if(part == 0 && strcmp(line, "PLAYER\n") != 0 && strcmp(line, "TARGET\n") != 0){
            circ_draw(scene, line);
            continue;
        }
        if(strcmp(line,"PLAYER\n") != 0 && strcmp(line, "TARGET\n") != 0 && strcmp(line, "WALLS\n") != 0){
            rect_draw(scene, line);
        }
    }

    while(!sdl_is_done(scene)){
        sdl_render_scene(scene, list_init(1, free));
    }

    fclose(f);
    scene_free(scene);
}