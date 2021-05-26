#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "forces.h"
#include "polygon.h"
#include "body.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "collision.h"
#include "textbox.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;

const double BALL_POINTS = 16;
const double BALL_RADIUS = 20;
const double BALL_MASS = 50.0;
const vector_t BALL_START = (vector_t) {100.0, 350.0};
const double BALL_MAX_VEL = 400.0;
const double BALL_MIN_VEL = 300.0;
const rgb_color_t BALL_COLOR = (rgb_color_t) {0.388, 0.09, 0.663};
const double BALL_ELASTICITY = .5;

const rgb_color_t WHITE_COLOR = (rgb_color_t) {1, 1, 1};

const double TONGUE_SPEED = 1500;
const double TONGUE_WIDTH = 10;
const rgb_color_t TONGUE_COLOR = (rgb_color_t) {0.388, 0.09, 0.663};

const double TONGUE_CUTOFF_LENGTH = 300;

list_t *INTERACTABLES;

const double CURSOR_RADIUS = 15;
const rgb_color_t CURSOR_COLOR = {0.388, 0.09, 0.663};
const double CURSOR_MASS = 1;
const double CURSOR_CIRCLE_THICKNESS = 3.2;
const size_t CURSOR_CIRCLE_OUTER_NPOINTS = 60;
const size_t CURSOR_CIRCLE_INNER_NPOINTS = 40;
const size_t CURSOR_NUM_CROSSHAIRS = 4;
const double CURSOR_CROSSHAIR_LENGTH = 4;
const double CURSOR_DOT_RADIUS = 1;
const size_t CURSOR_DOT_NPOINTS = 10;

const double FRICTION_COEFFICIENT = 30;

const double GRAVITY = 3000;

const double TONGUE_FORCE = 3000;

// SARAH'S CONSTANT DON'T TOUCH
// const double FRICTION_COEFFICIENT = 10;
// const double GRAVITY = 100;
// const double TONGUE_FORCE = 200;

rgb_color_t WALL_COLOR = {.5, .5, .5};

bool player_added = false;

bool clicked = false;

/**
 * Makes the body_t for the closed shape that is the outside of the cursor
 * around location and adds it to scene.
 * 
 * @param scene the scene to add the cursor outside to
 * @param center the center around which to create the cursor outside
 */ 
void draw_cursor_outside(scene_t *scene, vector_t center) {
    list_t *outside_points = list_init(CURSOR_CIRCLE_OUTER_NPOINTS + CURSOR_CIRCLE_INNER_NPOINTS, (free_func_t) vec_free);

    // Outer part of circle
    for (size_t i = 0; i <= CURSOR_CIRCLE_OUTER_NPOINTS; i++) {
        vector_t *v = malloc(sizeof(vector_t));
        double angle = i * 2 * M_PI / CURSOR_CIRCLE_OUTER_NPOINTS;
        *v = (vector_t) {center.x + CURSOR_RADIUS* cos(angle), center.y + CURSOR_RADIUS * sin(angle)};
        list_add(outside_points, v);
    }

    // Inner part of circle with crosshairs every CURSOR_NUM_CROSSHAIRS points
    double inner_radius = CURSOR_RADIUS - CURSOR_CIRCLE_THICKNESS;
    double shorter_radius = inner_radius - CURSOR_CROSSHAIR_LENGTH;
    for (size_t i = 0; i <= CURSOR_CIRCLE_INNER_NPOINTS; i++) {
        vector_t *v = malloc(sizeof(vector_t));
        double angle = i * 2 * M_PI / CURSOR_CIRCLE_INNER_NPOINTS;
        if (i % (CURSOR_CIRCLE_INNER_NPOINTS / CURSOR_NUM_CROSSHAIRS) == 0) {
            *v = (vector_t) {center.x + shorter_radius * cos(angle), center.y + shorter_radius * sin(angle)};
        }
        else {
            *v = (vector_t) {center.x + inner_radius * cos(angle), center.y + inner_radius * sin(angle)};
        }
        list_add(outside_points, v);
    }
    char *c = malloc(1);
    *c = 'C';
    body_t *body = body_init_with_info(outside_points, CURSOR_MASS, CURSOR_COLOR, c, (free_func_t) free);
    scene_add_body(scene, body);
    body_set_passive_rotation(body, M_PI / 4);
}

/**
 * Makes the body_t for the dot inside the cursor at location and adds it to scene.
 * 
 * @param scene the scene to add the cursor dot to
 * @param center the center around which to create the cursor dot
 */ 
void draw_cursor_dot(scene_t *scene, vector_t center) {
    list_t *dot_points = list_init(CURSOR_DOT_NPOINTS, (free_func_t) vec_free);

    for (size_t i = 0; i < CURSOR_DOT_NPOINTS; i++) {
        vector_t *v = malloc(sizeof(vector_t));
        double angle = i * 2 * M_PI / CURSOR_DOT_NPOINTS;
        *v = (vector_t) {center.x + CURSOR_DOT_RADIUS * cos(angle), center.y + CURSOR_DOT_RADIUS * sin(angle)};
        list_add(dot_points, v);
    }
    char *c = malloc(1);
    *c = 'I';
    scene_add_body(scene, body_init_with_info(dot_points, CURSOR_MASS, CURSOR_COLOR, c, (free_func_t) free));
}

body_t *rect_gen(scene_t *scene, double width, double height, double mass, vector_t center, rgb_color_t color, char *c, double rotation, char *image_name){
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

    polygon_rotate(rect_pts, rotation, center);
    body_t *bod = body_init_with_info(rect_pts, mass, color, c, free);
    if (image_name != NULL) {
        body_add_image(bod, image_init(image_name, (vector_t) {width, height}, rotation * 180 / M_PI));
    }
    scene_add_body(scene, bod);
    return bod;
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

bool cursor_is_outside(body_t *body){
    vector_t center = body_get_centroid(body);
    if(center.x > MAX_X || center.x < MIN_X){
        return true;
    }
    if(center.y > MAX_Y || center.y < MIN_Y){
        return true;
    }
    return false;
}

void surface_friction(body_t *body1, body_t *body2, vector_t axis, void *aux){    
    vector_t gravity = (vector_t) {0, -1 * body_get_mass(body1)};
    vector_t perp_axis = (vector_t) {axis.y, -1 * axis.x};

    double mult = vec_dot(gravity, axis);

    double friction = FRICTION_COEFFICIENT * fabs(mult);

    vector_t velocity = body_get_velocity(body1);

    vector_t vel_proj = vec_multiply(vec_dot(velocity, perp_axis), perp_axis);
    vel_proj = (vector_t) {-1 * vel_proj.x, vel_proj.y};
    vector_t friction_force = vec_multiply(friction, vel_proj);

    body_add_force(body1, friction_force);
}

void tongue_interaction(body_t *body1, body_t *body2, vector_t axis, void *aux){
    scene_t *scene = (scene_t *) aux;

    if(body_is_removed(body2) || (scene_get_body(scene, find_body_in_scene(scene, 'E', scene_bodies(scene))) == body2 && clicked)){
        return;
    }
    
    double index = find_body_in_scene(scene, 'L', scene_bodies(scene));
    if(index != -1){ 
        scene_remove_body(scene, index);
    }

    vector_t body1_cen = body_get_centroid(body1);
    vector_t body2_cen = body_get_centroid(body2);

    vector_t difference = vec_subtract(body2_cen, body1_cen);

    vector_t center = vec_multiply(.5, vec_add(body1_cen, body2_cen));
    
    double distance = sqrt(difference.x * difference.x + difference.y * difference.y);
    if(distance > TONGUE_CUTOFF_LENGTH || cursor_is_outside(body2)){
        double index = find_body_in_scene(scene, 'T', scene_bodies(scene));
        if(index != -1){ 
            scene_remove_body(scene, index);
        }
        index = find_body_in_scene(scene, 'L', scene_bodies(scene));
        if(index != -1){ 
            scene_remove_body(scene, index);
        }
        return;
    }
    double angle = acos(difference.y / distance);

    if(difference.x > 0){
        angle = 2 * M_PI - angle;
    }

    distance = fabs(distance);

    char *c = malloc(1);
    *c = 'L';
    rect_gen(scene, TONGUE_WIDTH, distance, INFINITY, center, TONGUE_COLOR, c, angle, NULL);    
}

body_t *circle_gen(scene_t *scene, size_t points, vector_t start, double radius, double mass, rgb_color_t color, char *c, bool add, char *image_name, vector_t image_dimensions){
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
    if (image_name != NULL) {
        body_add_image(ball_bod, image_init(image_name, image_dimensions, 0.0));
    }
    if (add){
        scene_add_body(scene, ball_bod);
    }
    return ball_bod;
}

void freeze_tongue_end(body_t *tongue, body_t *wall, vector_t axis, void *aux) {
    scene_t *scene = (scene_t *) aux;
    double index_body = find_body_in_scene(scene, 'T', scene_bodies(scene));
    double index_player = find_body_in_scene(scene, 'P', scene_bodies(scene));
    double index_goal = find_body_in_scene(scene, 'E', scene_bodies(scene));

    char *c = malloc(1);
    *c = 'T';
    vector_t center = body_get_centroid(tongue);
    body_t *new_tongue = circle_gen(scene, 10, center, TONGUE_WIDTH / 2, INFINITY, TONGUE_COLOR, c, false, NULL, VEC_ZERO);
    body_t *player = scene_get_body(scene, index_player);

    //create_interaction(scene, player, new_tongue, (collision_handler_t) tongue_interaction, scene, NULL);
    //create_tongue_force(scene, TONGUE_FORCE, player, new_tongue, INTERACTABLES);

    body_t *goal = scene_get_body(scene, index_goal);
    if(find_collision(body_get_shape(new_tongue), body_get_shape(goal)).collided){
        create_tongue_force(scene, TONGUE_FORCE, player, goal, INTERACTABLES);
        create_interaction(scene, player, goal, (collision_handler_t) tongue_interaction, scene, NULL);
    }
    else{
        create_interaction(scene, player, new_tongue, (collision_handler_t) tongue_interaction, scene, NULL);
        create_tongue_force(scene, TONGUE_FORCE, player, new_tongue, INTERACTABLES);
        scene_add_body(scene, new_tongue);
    }

    if(index_body != -1){ 
        scene_remove_body(scene, index_body);
    }
}

vector_t tongue_direction(body_t *body1, body_t *body2){
    vector_t body1_center = body_get_centroid(body1);
    vector_t body2_center = body_get_centroid(body2);

    vector_t difference = vec_subtract(body2_center, body1_center);
    double distance = sqrt(difference.x * difference.x + difference.y * difference.y);

    return vec_multiply(1 / distance, difference);
}

void create_viable_player_collisions(scene_t *scene, body_t *player){
    for(size_t i = 0; i < list_size(INTERACTABLES); i++){
        create_physics_collision(scene, BALL_ELASTICITY, player, list_get(INTERACTABLES, i));
        create_collision(scene, player, (body_t *) list_get(INTERACTABLES, i), (collision_handler_t) surface_friction, scene, NULL);
    }
}

void create_viable_tongue_collisions(scene_t *scene, body_t *tongue){
    for(size_t i = 0; i < list_size(INTERACTABLES); i++){
        body_t *body = list_get(INTERACTABLES, i);
        create_collision(scene, tongue, body, (collision_handler_t) freeze_tongue_end, scene, NULL);
    }
}

void bouncy_wall_gen(scene_t *scene){
    char *c = malloc(1);
    *c = 'W';
    body_t *left_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                 (vector_t) {-MAX_X, MAX_Y / 2}, (rgb_color_t) {1, 1, 1}, c, 0, NULL);
    list_add(INTERACTABLES, left_wall);
    //create_physics_collision(scene, BALL_ELASTICITY, left_wall, player);

    c = malloc(1);
    *c = 'W';
    body_t *right_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                  (vector_t) {2 * MAX_X, MAX_Y / 2}, (rgb_color_t) {1, 1, 1}, c, 0, NULL);
    list_add(INTERACTABLES, right_wall);
    //create_physics_collision(scene, BALL_ELASTICITY, right_wall, player);
    
    c = malloc(1);
    *c = 'W';
    body_t *top_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                  (vector_t) {MAX_X / 2, 2 * MAX_Y}, (rgb_color_t) {1, 1, 1}, c, 0, NULL);
    list_add(INTERACTABLES, top_wall);
    //create_physics_collision(scene, BALL_ELASTICITY, top_wall, player);
}

void tongue_removal(scene_t *scene){
    int index = scene_bodies(scene) - 1;
    while(index != -1){
        if(((char *) body_get_info(scene_get_body(scene, index)))[0] == 'L'){
            scene_remove_body(scene, index);
        }
        index--;
    }
}

void on_key(char key, key_event_type_t type, double held_time, void *scene, vector_t loc) {
    loc = (vector_t) {loc.x, MAX_Y - loc.y};
    body_t *cursor_out = scene_get_body((scene_t *) scene, find_body_in_scene(scene, 'C', scene_bodies(scene)));
    body_t *cursor_dot = scene_get_body((scene_t *) scene, find_body_in_scene(scene, 'I', scene_bodies(scene)));
    body_t *player = scene_get_body((scene_t *) scene, find_body_in_scene(scene, 'P', scene_bodies(scene)));
    body_t *reset_button = scene_get_body((scene_t *) scene, find_body_in_scene(scene, 'R', scene_bodies(scene)));
    body_t *target = scene_get_body((scene_t *) scene, find_body_in_scene(scene, 'E', scene_bodies(scene)));
    
    if (type == KEY_PRESSED) {
        switch (key) {
            case MOUSE_CLICK: {

                if(find_collision(body_get_shape(cursor_dot), body_get_shape(reset_button)).collided){
                    body_remove(reset_button);
                    break;
                }

                char *c = malloc(1);
                *c = 'T';
                body_t *tongue_end = circle_gen(scene, 3, body_get_centroid(player), 3, INFINITY, TONGUE_COLOR, c, true, NULL, VEC_ZERO);
                vector_t direction = tongue_direction(player, cursor_dot);
                body_set_velocity(tongue_end, vec_multiply(TONGUE_SPEED, direction));
                create_interaction(scene, player, tongue_end, (collision_handler_t) tongue_interaction, scene, NULL);
                create_viable_tongue_collisions(scene, tongue_end);
                clicked = false;
                break;
            }
        }
    }
    if(type == KEY_RELEASED){
        switch(key) {
            case MOUSE_CLICK: {
                clicked = true;
                double index = find_body_in_scene(scene, 'T', scene_bodies(scene));
                if(index != -1){
                    scene_remove_body(scene, index);
                }

                // CONSTRUCTION ZONE AHEAD

                list_t *forces = scene_get_forces(scene);
                for(size_t i = 0; i < list_size(forces); i++){
                    force_t *force = list_get(forces, i);
                    force_creator_t force_func = (force_creator_t) force_get_forcer(force);
                    if(force_func != (force_creator_t) calc_tongue_force){
                        continue;
                    }
                    list_t *bodies = force_get_bodies(force);
                    body_t *body1 = (body_t *) list_get(bodies, 0);
                    body_t *body2 = (body_t *) list_get(bodies, 1);
                    if((body1 == player && body2 == target) || (body1 == target && body2 == player)){
                        list_remove(forces, i);
                    }
                }

                // END OF CONSTRUCTION ZONE

                tongue_removal(scene);

                break;
            }
        }
    }
    if (type == MOUSE_ENGAGED) {
        switch (key) {
            case MOUSE_MOVED: {
                body_set_centroid(cursor_out, loc);
                body_set_centroid(cursor_dot, loc);
                break;
            }
        }
    }
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
    char *image_name = malloc(20);
    vector_t dimensions;
    if(strtod(list_get(list, 8), &holder) == 0){
        *c = 'P';
        image_name = "images/tarzan-ball.png";
        dimensions = (vector_t) {60, 60};
    }
    else{
        *c = 'E';
        image_name = "images/target-image.png";
        dimensions = (vector_t) {40, 40};
    }

    body_t *body = circle_gen(scene, strtod(list_get(list, 0), &holder), (vector_t) {strtod(list_get(list, 1),
            &holder), strtod(list_get(list, 2), &holder)}, strtod(list_get(list, 3), &holder), strtod(list_get(list, 4),
            &holder), (rgb_color_t) {strtod(list_get(list, 5), &holder), strtod(list_get(list, 6), &holder),
            strtod(list_get(list, 7), &holder)}, c, true, image_name, dimensions);
    if(player_added){
        list_add(INTERACTABLES, body);
    }
    player_added = true;
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

    char *image = malloc(20);
    char *c = malloc(1);
    if(strtod(list_get(list, 8), &holder) == 0){
        *c = 'W';
        sprintf(image, "images/floor.png");
    }
    else{
        *c = 'K';
        sprintf(image, "images/lava.png");
    }

    body_t *body = rect_gen(scene, strtod(list_get(list, 0), &holder), strtod(list_get(list, 1), &holder),
            strtod(list_get(list, 2), &holder), (vector_t) {strtod(list_get(list, 3), &holder), strtod(list_get(list, 4),
            &holder)}, (rgb_color_t) {strtod(list_get(list, 5), &holder), strtod(list_get(list, 6), &holder),
            strtod(list_get(list, 7), &holder)}, c, strtod(list_get(list, 9), &holder), image);
    list_add(INTERACTABLES, body);

    if(*c == 'K'){
        create_half_destruction(scene, body, scene_get_body(scene, find_body_in_scene(scene, 'P', scene_bodies(scene))));
    }
}

int level_init(scene_t *scene, char *file_name) {
    FILE *f = fopen(file_name, "r");

    int bodies = -3;
    char *line = malloc(200);
    int part = 0;

    while(fgets(line, 200, f)){
        if(strcmp(line, "WALLS\n") == 0){
            part = 1;
        }
        if(part == 0 && strcmp(line, "PLAYER\n") != 0 && strcmp(line, "TARGET\n") != 0) {
            circ_draw(scene, line);
            continue;
        }
        if(strcmp(line,"PLAYER\n") != 0 && strcmp(line, "TARGET\n") != 0 && strcmp(line, "WALLS\n") != 0) {
            rect_draw(scene, line);
        }
        bodies++;
    }

    fclose(f);
    // free line, a char * initialized earlier in this method?
    return bodies;
}

// Returns the name of the text file for the level_num-th level
char *get_level_name_from_num(size_t level_num) {
    char *level_name = malloc(20);
    sprintf(level_name, "levels/level_%zu.txt", level_num);
    return level_name;
}

// Sets up everything for a level outlined in the textfile at level_name and returns
// a pointer to the set up scene_t
scene_t *set_up_level(size_t level_num) {
    scene_t *scene = scene_init();
    char *background_name = malloc(30);
    sprintf(background_name, "images/background.png");
    scene_set_background(scene, background_name, (vector_t) {MAX_X - MIN_X, MAX_Y - MIN_Y});
    INTERACTABLES = list_init(5, (free_func_t) body_free);
    
    char *level_name = get_level_name_from_num(level_num);
    level_init(scene, level_name);
    free(level_name);
    sdl_init((vector_t) {MIN_X, MIN_Y}, (vector_t) {MAX_X, MAX_Y});

    draw_cursor_outside(scene, VEC_ZERO);
    draw_cursor_dot(scene, VEC_ZERO);

    bouncy_wall_gen(scene);

    create_viable_player_collisions(scene, scene_get_body(scene, 0));
    create_viable_player_collisions(scene, scene_get_body(scene, 1));
    create_half_destruction(scene, scene_get_body(scene, 0), scene_get_body(scene, 1));

    create_universal_gravity(scene, GRAVITY, scene_get_body(scene, 0), INTERACTABLES);
    create_universal_gravity(scene, GRAVITY, scene_get_body(scene, 1), INTERACTABLES);

    char *c = malloc(1);
    *c = 'R';
    body_t *restart_button = rect_gen(scene, 100, 50, INFINITY, (vector_t) {60, 465}, (rgb_color_t) {0.1, 0.1, 0.1}, c, 0, NULL);
    body_add_image(restart_button, image_init("images/button.png", (vector_t) {110, 60}, 0));

    return scene;
}

list_t *make_tutorial(){
    list_t *ret = list_init(2, (free_func_t) textbox_free);
    // x, y, width, height
    textbox_t *one = textbox_init(100, 100, 800, 40, "Press and hold the mouse to shoot your tongue and move!", 
                                      TTF_OpenFont("fonts/karvwood.otf", 12), (SDL_Color) {255, 0, 255});
    list_add(ret, one);
    textbox_t *two = textbox_init(10, 25, 100, 30, "RESET", 
                                      TTF_OpenFont("fonts/arial.ttf", 12), (SDL_Color) {255, 255, 255});
    list_add(ret, two);
    return ret;
}

list_t *default_tbs(){
    list_t *ret = list_init(2, (free_func_t) textbox_free);
    // x, y, width, height
    textbox_t *two = textbox_init(10, 25, 100, 30, "RESET", 
                                      TTF_OpenFont("fonts/arial.ttf", 12), (SDL_Color) {255, 255, 255});
    list_add(ret, two);
    return ret;
}

list_t *YOU_DIED(){
    list_t *ret = list_init(2, (free_func_t) textbox_free);
    // x, y, width, height
    textbox_t *two = textbox_init(0, 15, 1000, 500, "YOU DIED NERD", 
                                      TTF_OpenFont("fonts/arial.ttf", 20), (SDL_Color) {255, 0, 0});
    list_add(ret, two);
    return ret;
}

int main(int argc, char *argv[]) {
    size_t current_level = 1;
    scene_t *scene = set_up_level(current_level);
    list_t *textboxes;

    bool died = false;

    int count = 0;

    while (!sdl_is_done(scene)) {
        sdl_on_key((key_handler_t) on_key);
        
        // if first level, then display tutorial text
        if(current_level == 1) {
            textboxes = make_tutorial();
        }
        else {
            if(died && count < 50){
                textboxes = YOU_DIED();
                count++;
            }
            else{
                count = 0;
                died = false;
                textboxes = default_tbs();
            }
        }

        // reset button, restarts the level, red square
        if (find_body_in_scene(scene, 'R', scene_bodies(scene)) == -1){
            player_added = false;
            scene_free(scene);
            sdl_free();
            scene = set_up_level(current_level);
        }

        // win condition
        // if the target disappears, as there is half dest between player and target
        if (find_body_in_scene(scene, 'E', scene_bodies(scene)) == -1){
            current_level++;
            if(current_level > 3){
                // Need a win screen here. Maybe a celebration picture of Tarzan
                break;
            }
            player_added = false;
            scene_free(scene);
            sdl_free();
            scene = set_up_level(current_level);
        }
        
        // loss condition, player is no longer there
        if(find_body_in_scene(scene, 'P', scene_bodies(scene)) == -1){
            died = true;
            player_added = false;
            scene_free(scene);
            sdl_free();
            scene = set_up_level(current_level);
        }

        scene_tick(scene, time_since_last_tick());
        sdl_render_scene(scene, textboxes);
    }

    scene_free(scene);
    sdl_free();
}
