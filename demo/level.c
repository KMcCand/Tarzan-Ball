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

const rgb_color_t CURSOR_COLOR = {0.388, 0.09, 0.663};

rgb_color_t WALL_COLOR = {.5, .5, .5};

double RECTANGLE_NUM = 0;

list_t *RECTANGLES;

bool cursor_val = false;

bool player_in = false;

bool target_in = false;

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

typedef struct circle{
    scene_t *scene;
    size_t points;
    vector_t start;
    double radius;
    double mass;
    rgb_color_t color;
    char c;
}circle_t;

circle_t *player;

circle_t *target;

circle_t *circle_init(scene_t *scene, size_t points, vector_t start, double radius, double mass, rgb_color_t color, char c){
    circle_t *circle = malloc(sizeof(circle_t));
    circle->scene = malloc(sizeof(scene_t *));
    circle->points = points;
    circle->start = start;
    circle->radius = radius;
    circle->mass = mass;
    circle->color = color;
    circle->c = c;
    return circle;
}

char *circle_string_info(circle_t *circle){
    char *str = malloc(200);
    double char_trans;
    if(circle->c == 'P'){
        char_trans = 0;
    }
    else{
        char_trans = 1;
    }
    sprintf(str, "%.2zu  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f", circle->points, circle->start.x, circle->start.y, circle->radius, circle->mass, circle->color.r, circle->color.g, circle->color.b, char_trans);
    return str;    
}

typedef struct rect{
    scene_t *scene;
    double width;
    double height;
    double mass;
    vector_t center;
    rgb_color_t color;
    char c;
    double rotate;
}rect_t;

rect_t *rect_init(scene_t *scene, double width, double height, double mass, vector_t center, rgb_color_t color, char c, double rotate){
    rect_t *rect = malloc(sizeof(rect_t));
    rect->scene = malloc(sizeof(scene_t *));
    //rect->c = malloc(sizeof(char *));
    rect->c = c;
    rect->scene = scene;
    rect->width = width;
    rect->height = height;
    rect->mass = mass;
    rect->center = center;
    rect->color = color;
    rect->c = c;
    rect->rotate = rotate;
    return rect;
}

void rect_add(rect_t *rect){
    rect_gen(rect->scene, rect->width, rect->height, rect->mass, rect->center, rect->color, rect->c, rect->rotate);
}

void rect_width_increase(rect_t *rect){
    if(rect->width < 1000){
        rect->width += 10;
    }
}

void rect_height_increase(rect_t *rect){
    if(rect->height < 1000)
    rect->height += 10;
}

void rect_width_decrease(rect_t *rect){
    if(rect->width > 20){
        rect->width -= 10;
    }
}

void rect_height_decrease(rect_t *rect){
    if(rect->height > 20){
        rect->height -= 10;
    }
}

void rect_center(rect_t *rect, vector_t center){
    rect->center = center;
}

void rect_rotate(rect_t *rect){
    rect->rotate += M_PI / 8;
}

char rect_get_char(rect_t *rect){
    return rect->c;
}

char *rect_string_info(rect_t *rect){
    char *str = malloc(200);
    double char_trans;
    if(rect->c == 'W'){
        char_trans = 0;
    }
    else{
        char_trans = 1;
    }
    sprintf(str, "%.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f", rect->width, rect->height, rect->mass, rect->center.x, rect->center.y, rect->color.r, rect->color.g, rect->color.b, char_trans, rect->rotate);
    return str;
}

body_t *circle_gen(scene_t *scene, size_t points, vector_t start, double radius, double mass, rgb_color_t color, char *c, bool add){
    list_t *ball_points = list_init(points, (free_func_t) vec_free);
    for (size_t i = 0; i < points; i++) {
        double angle = 2 * M_PI * i / points; 
        vector_t *v = malloc(sizeof(*v));
        *v = (vector_t) {start.x + radius * cos(angle), 
                         start.y + radius * sin(angle)};
        list_add(ball_points, v);
    }
    polygon_rotate(ball_points, M_PI / points, start);
    body_t *ball_bod = body_init_with_info(ball_points, mass, color, c, (free_func_t) free);
    if(add){
        scene_add_body(scene, ball_bod);
    }
    return ball_bod;
}

vector_t tongue_direction(body_t *body1, body_t *body2){
    vector_t body1_center = body_get_centroid(body1);
    vector_t body2_center = body_get_centroid(body2);

    vector_t difference = vec_subtract(body2_center, body1_center);
    double distance = sqrt(difference.x * difference.x + difference.y * difference.y);

    return vec_multiply(1 / distance, difference);
}

void starting_rect_gen(scene_t *scene, vector_t start){
    rect_t *rect = rect_init(scene, 100, 50, INFINITY, start, WALL_COLOR, 'W', 0);
    rect_add(rect);
    list_add(RECTANGLES, rect);
}

double find_body_in_scene(scene_t *scene, char type, int start){
    double index = start - 1;
    while(((char *) body_get_info(scene_get_body(scene, index)))[0] != type){
        index--;
        if(index <= -1){
            return -1;
        }
    }
    return index;
}

void on_key(char key, key_event_type_t type, double held_time, void *scene, vector_t loc) {
    loc = (vector_t) {loc.x, MAX_Y - loc.y};
    body_t *cursor = scene_get_body((scene_t *) scene, RECTANGLE_NUM);
    rect_t *current_rect = list_get(RECTANGLES, RECTANGLE_NUM);
    
    if (type == KEY_PRESSED) {
        switch (key) {
            case MOUSE_CLICK: {
                printf("clicked mouse\n");
                if(find_body_in_scene(scene, 'P', scene_bodies(scene)) == RECTANGLE_NUM){
                    player = circle_init(scene, 16, loc, 20, 50.00, (rgb_color_t) {0.388, 0.09, 0.663}, 'P');
                    player_in = true;
                }
                if(find_body_in_scene(scene, 'E', scene_bodies(scene)) == RECTANGLE_NUM){
                    target = circle_init(scene, 16, loc, 20, 50.00, (rgb_color_t) {1, 0, 0}, 'E');
                    target_in = true;
                }
                if(cursor_val){
                    //double cursor_index = find_body_in_scene(scene, 'C', scene_bodies(scene));
                    for(size_t i = 0; i < scene_bodies(scene); i++){
                        if(i == RECTANGLE_NUM){
                            continue;
                        }
                        if(find_collision(body_get_shape(cursor), body_get_shape(scene_get_body(scene, i))).collided){
                            if(find_body_in_scene(scene, 'P', scene_bodies(scene)) == i){
                                double ind = find_body_in_scene(scene, 'C', scene_bodies(scene));
                                scene_remove_body(scene, ind);
                                scene_remove_body(scene, i);
                                cursor_val = false;
                                char *c = malloc(1);
                                *c = 'P';
                                circle_gen(scene, 16, loc, 20, 50.00, (rgb_color_t) {0.388, 0.09, 0.663}, c, true);
                                RECTANGLE_NUM --;
                                player_in = false;
                                break;
                            }
                            else if(find_body_in_scene(scene, 'E', scene_bodies(scene)) == i){
                                double ind = find_body_in_scene(scene, 'C', scene_bodies(scene));
                                scene_remove_body(scene, ind);
                                scene_remove_body(scene, i);
                                cursor_val = false;
                                char *c = malloc(1);
                                *c = 'P';
                                circle_gen(scene, 16, loc, 20, 50.00, (rgb_color_t) {1, 0, 0}, c, true);
                                RECTANGLE_NUM --;
                                target_in = false;
                                break;
                            }
                            else{
                                double ind = find_body_in_scene(scene, 'C', scene_bodies(scene));
                                scene_remove_body(scene, ind);
                                scene_remove_body(scene, i);
                                cursor_val = false;
                                rect_add(list_get(RECTANGLES, i));
                                RECTANGLE_NUM --;
                            }
                            break;
                        }
                    }
                    break;
                }
                RECTANGLE_NUM += 1;
                starting_rect_gen(scene, loc);
                break;
            }
            case RIGHT_ARROW:{
                rect_width_increase(current_rect);
                rect_add(current_rect);
                scene_remove_body(scene, RECTANGLE_NUM);
                break;
            }
            case LEFT_ARROW:{
                rect_width_decrease(current_rect);
                rect_add(current_rect);
                scene_remove_body(scene, RECTANGLE_NUM);
                break;
            }
            case UP_ARROW:{
                rect_height_increase(current_rect);
                rect_add(current_rect);
                scene_remove_body(scene, RECTANGLE_NUM);
                break;
            }
            case DOWN_ARROW:{
                rect_height_decrease(current_rect);
                rect_add(current_rect);
                scene_remove_body(scene, RECTANGLE_NUM);
                break;
            }
            case R_KEY:{
                rect_rotate(current_rect);
                rect_add(current_rect);
                scene_remove_body(scene, RECTANGLE_NUM);
                break;
            }
            case S_KEY: {
                if(cursor_val){
                    break;
                }
                scene_remove_body(scene, RECTANGLE_NUM);
                char *c = malloc(1);
                *c = 'C';
                circle_gen(scene, 10, loc, 10, INFINITY, (rgb_color_t) {1, 0, 0}, c, true);
                cursor_val = true;
                break;
            }
            case W_KEY: {
                if(!cursor_val){
                    break;
                }
                double ind = find_body_in_scene(scene, 'C', scene_bodies(scene));
                starting_rect_gen(scene, loc);
                scene_remove_body(scene, ind);
                cursor_val = false;
                break;
            }
            case P_KEY: {
                if(player_in){
                    break;
                }
                scene_remove_body(scene, RECTANGLE_NUM);
                char *c = malloc(1);
                *c = 'P';
                circle_gen(scene, 16, loc, 20, 50.00, (rgb_color_t) {0.388, 0.09, 0.663}, c, true);
                cursor_val = false;
                break;
            }
            case D_KEY: {
                if(scene_bodies(scene) <= 1){
                    break;
                }
                scene_remove_body(scene, RECTANGLE_NUM);
                RECTANGLE_NUM --;
                break;
            }
            case T_KEY: {
                if(target_in){
                    break;
                }
                scene_remove_body(scene, RECTANGLE_NUM);
                char *c = malloc(1);
                *c = 'E';
                circle_gen(scene, 16, loc, 20, 50.00, (rgb_color_t) {1, 0, 0}, c, true);
                cursor_val = false;
                break;
            }
            case K_KEY: {
                scene_remove_body(scene, RECTANGLE_NUM);
                rect_t *rect = rect_init(scene, 100, 50, INFINITY, loc, (rgb_color_t) {1, 0, 0}, 'K', 0);
                list_remove(RECTANGLES, RECTANGLE_NUM);
                rect_add(rect);
                list_add(RECTANGLES, rect);
                break;
            }

        }
    }
    if (type == MOUSE_ENGAGED) {
        switch (key) {
            case MOUSE_MOVED: {
                //body_redefine_centroid(cursor, loc);
                rect_center(current_rect, loc);
                body_set_centroid(cursor, loc);
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    RECTANGLES = list_init(3, free);
    sdl_init((vector_t) {MIN_X, MIN_Y}, (vector_t) {MAX_X, MAX_Y}, list_init(2, free), list_init(2, free), list_init(2, free));
    scene_t *scene = scene_init();

    starting_rect_gen(scene, (vector_t) {500, 250});
    double dt = 0;

    while (!sdl_is_done(scene)) {
        dt = time_since_last_tick();
        sdl_on_key((key_handler_t) on_key);

        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }

    int player_int = 0;
    if(player != NULL){
        player_int = 1;
    }

    int target_int = 0;
    if(target != NULL){
        target_int = 1;
    }

    FILE *f = fopen("levels/level_TESTING.txt", "w");
    fprintf(f, "PLAYER\n");
    if(player != NULL){
        fprintf(f, "%s\n", circle_string_info(player));
    }
    fprintf(f, "TARGET\n");
    if(target != NULL){
        fprintf(f, "%s\n", circle_string_info(target));
    }
    fprintf(f, "WALLS\n");
    for(size_t i = 0; i < list_size(RECTANGLES) - 1 - target_int - player_int; i++){
        fprintf(f, "%s\n", rect_string_info(list_get(RECTANGLES, i)));
    }

    fclose(f);

    scene_free(scene);
}