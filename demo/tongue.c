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

const double BALL_POINTS = 16;
const double BALL_RADIUS = 20;
const double BALL_MASS = 50.0;
const vector_t BALL_START = (vector_t) {100.0, 350.0};
const double BALL_MAX_VEL = 400.0;
const double BALL_MIN_VEL = 300.0;
const rgb_color_t BALL_COLOR = (rgb_color_t) {0.388, 0.09, 0.663};
const double BALL_ELASTICITY = .5;

const rgb_color_t WHITE_COLOR = (rgb_color_t) {1, 1, 1};

const double TONGUE_SPEED = 2000;
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

rgb_color_t WALL_COLOR = {.5, .5, .5};

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
    scene_add_body(scene, body_init_with_info(outside_points, CURSOR_MASS, CURSOR_COLOR, c, (free_func_t) free));
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
    *c = 'C';
    scene_add_body(scene, body_init_with_info(dot_points, CURSOR_MASS, CURSOR_COLOR, c, (free_func_t) free));
}

body_t *rect_gen(scene_t *scene, double width, double height, double mass, vector_t center, rgb_color_t color, char *c, double rotate){
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
    body_t *bod = body_init_with_info(rect_pts, mass, color, c, free);
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

    if(body_is_removed(body2)){
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
    rect_gen(scene, TONGUE_WIDTH, distance, INFINITY, center, TONGUE_COLOR, c, angle);    
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

void freeze_tongue_end(body_t *tongue, body_t *wall, vector_t axis, void *aux) {
    scene_t *scene = (scene_t *) aux;
    double index_body = find_body_in_scene(scene, 'T', scene_bodies(scene));
    double index_player = find_body_in_scene(scene, 'P', scene_bodies(scene));
    double index_goal = find_body_in_scene(scene, 'E', scene_bodies(scene));

    char *c = malloc(1);
    *c = 'T';
    vector_t center = body_get_centroid(tongue);

    body_t *new_tongue = circle_gen(scene, 10, center, TONGUE_WIDTH / 2, INFINITY, TONGUE_COLOR, c, false);
    body_t *player = scene_get_body(scene, index_player);

    //create_interaction(scene, player, new_tongue, (collision_handler_t) tongue_interaction, scene, NULL);
    //create_tongue_force(scene, TONGUE_FORCE, player, new_tongue, INTERACTABLES);

    body_t *goal = scene_get_body(scene, index_goal);
    if(find_collision(body_get_shape(new_tongue), body_get_shape(goal)).collided){
        create_tongue_force(scene, TONGUE_FORCE, player, scene_get_body(scene, index_goal), INTERACTABLES);
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
        //create_collision(scene, player, (body_t *) list_get(INTERACTABLES, i), (collision_handler_t) impulse_collision, scene, NULL);
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
                                 (vector_t) {-MAX_X, MAX_Y / 2}, (rgb_color_t) {1, 1, 1}, c, 0);
    list_add(INTERACTABLES, left_wall);
    //create_physics_collision(scene, BALL_ELASTICITY, left_wall, player);

    c = malloc(1);
    *c = 'W';

    body_t *right_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                  (vector_t) {2 * MAX_X, MAX_Y / 2}, (rgb_color_t) {1, 1, 1}, c, 0);
    list_add(INTERACTABLES, right_wall);
    //create_physics_collision(scene, BALL_ELASTICITY, right_wall, player);
    
    c = malloc(1);
    *c = 'W';
    body_t *top_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                  (vector_t) {MAX_X / 2, 2 * MAX_Y}, (rgb_color_t) {1, 1, 1}, c, 0);
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
    body_t *cursor_out = scene_get_body((scene_t *) scene, 6);
    body_t *cursor_dot = scene_get_body((scene_t *) scene, 7);
    body_t *player = scene_get_body((scene_t *) scene, 0);
    
    if (type == KEY_PRESSED) {
        switch (key) {
            case MOUSE_CLICK: {
                char *c = malloc(1);
                *c = 'T';
                body_t *tongue_end = circle_gen(scene, 3, body_get_centroid(player), 3, INFINITY, WHITE_COLOR, c, true);
                vector_t direction = tongue_direction(player, cursor_dot);
                body_set_velocity(tongue_end, vec_multiply(TONGUE_SPEED, direction));
                create_interaction(scene, player, tongue_end, (collision_handler_t) tongue_interaction, scene, NULL);
                create_viable_tongue_collisions(scene, tongue_end);
                break;
            }
        }
    }
    if(type == KEY_RELEASED){
        switch(key) {
            case MOUSE_CLICK: {
                double index = find_body_in_scene(scene, 'T', scene_bodies(scene));
                if(index != -1){
                    scene_remove_body(scene, index);
                }
                tongue_removal(scene);
                break;
            }
        }
    }
    if (type == MOUSE_ENGAGED) {
        switch (key) {
            case MOUSE_MOVED: {
                //body_redefine_centroid(cursor, loc);
                body_set_centroid(cursor_out, loc);
                body_set_centroid(cursor_dot, loc);
                break;
            }
        }
    }
}

/**
* Adds an image name to images and its respective dimensions to dimensions.
*
* @param images the images list of char * types to add the name to
* @param dimensions the dimensions list of vector_t * types to add the dimensions to
* @param name the name of the image to add
* @param x the x dimension of the image
* @param y the y dimension of the image
*/
void add_image(list_t *images, list_t *dimensions, char *name, double x, double y) {
    char *image_name = malloc(strlen(name) + 1);
    strcpy(image_name, name);
    list_add(images, image_name);

    vector_t *dimension = malloc(sizeof(vector_t));
    dimension->x = x;
    dimension->y = y;
    list_add(dimensions, dimension);
}

int main(int argc, char *argv[]) {
    list_t *images = list_init(2, free);
    list_t *dimensions = list_init(2, free);

    add_image(images, dimensions, "demo/tarzan-eyes.png", 40, 40);
    add_image(images, dimensions, "demo/target-image.png", 40, 40);
    add_image(images, dimensions, "demo/ground.png", 200, 50);


    sdl_init((vector_t) {MIN_X, MIN_Y}, (vector_t) {MAX_X, MAX_Y}, images, dimensions);
    INTERACTABLES = list_init(5, (free_func_t) body_free);  
    scene_t *scene = scene_init();

    char *c = malloc(1);
    *c = 'P';
    body_t *player = circle_gen(scene, BALL_POINTS, BALL_START, BALL_RADIUS, BALL_MASS, BALL_COLOR, c, true); //player

    c = malloc(1);
    *c = 'E';
    body_t *finish = circle_gen(scene, BALL_POINTS, (vector_t) {400, 200}, BALL_RADIUS, BALL_MASS, (rgb_color_t) {1, 0, 0}, c, true); //target 
    list_add(INTERACTABLES, finish);

    c = malloc(1);
    *c = 'W';
    body_t *high_platform_tester = rect_gen(scene, 50, 200, INFINITY, (vector_t) {400, 350}, WALL_COLOR, c, M_PI / 2); //middle plat 
    list_add(INTERACTABLES, high_platform_tester);

    c = malloc(1);
    *c = 'W';
    body_t *starting_platform = rect_gen(scene, 50, 200, INFINITY, (vector_t) {100, 200}, WALL_COLOR, c, 5 * M_PI / 4); //starting platform
    list_add(INTERACTABLES, starting_platform);

    c = malloc(1);
    *c = 'W';
    body_t *wall = rect_gen(scene, 50, 500, INFINITY, (vector_t) {750, 250}, WALL_COLOR, c, 0); //wall
    list_add(INTERACTABLES, wall);

    c = malloc(1);
    *c = 'W';
    body_t *floor = rect_gen(scene, 100, 1000, INFINITY, (vector_t) {500, 0}, WALL_COLOR, c, M_PI / 2); //floor
    list_add(INTERACTABLES, floor);

    draw_cursor_outside(scene, VEC_ZERO);
    draw_cursor_dot(scene, VEC_ZERO);

    bouncy_wall_gen(scene);

    create_viable_player_collisions(scene, finish);

    create_universal_gravity(scene, GRAVITY, player, INTERACTABLES);
    create_universal_gravity(scene, GRAVITY, finish, INTERACTABLES);
    create_viable_player_collisions(scene, player);

    double dt = 0;

    while (!sdl_is_done(scene)) {
        dt = time_since_last_tick();
        sdl_on_key((key_handler_t) on_key);

        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }

    list_free(images);
    scene_free(scene);
    // sdl_free will free dimensions so we do not have to free that here
    sdl_free();
}
