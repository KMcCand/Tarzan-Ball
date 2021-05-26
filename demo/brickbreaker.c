#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"

const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 500.0;

const double PLAYER_WIDTH = 130;
const double PLAYER_HEIGHT = 30;
const vector_t PLAYER_START = (vector_t) {500.0, 10.0};
const double PLAYER_SPEED = 600;
const double PLAYER_MASS = INFINITY;
const rgb_color_t PLAYER_COLOR = (rgb_color_t) {1.0, 0.0, 0.0};

const double BALL_RADIUS = 5;
const double BALL_MASS = 5.0;
const size_t BALL_POINTS = 15;
const vector_t BALL_START = (vector_t) {500.0, 100.0};
const double BALL_MAX_VEL = 400.0;
const double BALL_MIN_VEL = 300.0;
const rgb_color_t BALL_COLOR = (rgb_color_t) {1.0, 0.0, 0.0};
const double BALL_ELASTICITY = 1;

const size_t BRICK_LAYERS = 4;
const double BRICK_HEIGHT = 30;
const double BRICKS_PER_LAYER = 12;
const double BRICK_SPACING = 5;
const double BRICK_MASS = INFINITY;

const double LIVES_STARTING_CENTER_X = 15.0;
const double LIVES_CENTER_Y = 60.0;
const rgb_color_t LIVES_COLOR = (rgb_color_t) {0.8, .25, .33};
const double LIVES_HORIZONTAL_SPACE = 5.0;
const double LIVES_WIDTH = 10.0;
const double LIVES_HEIGHT = 100.0;
const int LIVES_AT_START = 3;

// Generates a rectangle on scene with width, height, mass, center, and color. Takes
// info parameter c which tells what type of player this rectangle will be in the game
body_t *rect_gen(scene_t *scene, double width, double height, double mass, vector_t center, rgb_color_t color, char *c){
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

    body_t *bod = body_init_with_info(rect_pts, mass, color, c, free);

    scene_add_body(scene, bod);
    return bod;
}

// Generates the bricks for brickbreaker and puts them on scene. Brick appearance
// controlled by the constants starting with "BRICK_"
void brick_gen(scene_t *scene) {
    double brick_width = (MAX_X - (BRICKS_PER_LAYER + 1) * BRICK_SPACING) / BRICKS_PER_LAYER;
    body_t *ball = scene_get_body(scene, 1);
    char *c = malloc(1);
    *c = 'B';

    for (size_t layer = 0; layer < BRICK_LAYERS; layer++) {
        double y_lvl = MAX_Y - (BRICK_SPACING  * (layer + 1) + layer * BRICK_HEIGHT + .5 * BRICK_HEIGHT);
        for (size_t brick = 0; brick < BRICKS_PER_LAYER; brick++){
            // Method for generating rainbow colors found here: https://krazydad.com/tutorials/makecolors.php
            double angle = 2 * M_PI * brick / BRICKS_PER_LAYER + M_PI;
            rgb_color_t color = (rgb_color_t) {(sin(angle - M_PI/2) + 1)/2, (sin(angle - 7 * M_PI/6) + 1)/2, 
                                               (sin(angle - 11 * M_PI/6) + 1)/2};
            
            double x_lvl = BRICK_SPACING * (brick + 1) + brick * brick_width + .5 * brick_width;
            vector_t brick_center = (vector_t ) {x_lvl, y_lvl};
            body_t *brick_bod = rect_gen(scene, brick_width, BRICK_HEIGHT, BRICK_MASS, brick_center, color, c);
            create_physics_collision(scene, BALL_ELASTICITY, brick_bod, ball);
            create_half_destruction(scene, ball, brick_bod);
            c = malloc(1);
            *c = 'B';
        }
    }
}

// Sets the ball to a velocity with random x direction and positive y direction of random
// magnitudes as set by the program constants.
void set_ball_velocity(body_t *ball) {
    double x_vel = (double) rand() / (double) RAND_MAX * (BALL_MAX_VEL - BALL_MIN_VEL) + BALL_MIN_VEL;
    double neg = (double) rand() / (double) RAND_MAX;
    if (neg < 0.5) {
        x_vel = -x_vel;
    }
    body_set_velocity(ball, (vector_t) {x_vel, (double) rand() / (double) RAND_MAX * (BALL_MAX_VEL - BALL_MIN_VEL) + BALL_MIN_VEL});
}

// Generates the bouncing ball in brickbreaker on scene. If replace is true, replaces the second
// body in scene with the ball, if it is false, adds the ball to the scene
void ball_gen(scene_t *scene) {
    list_t *ball_points = list_init(BALL_POINTS, (free_func_t) vec_free);
    for (size_t i = 0; i < BALL_POINTS; i++) {
        double angle = 2 * M_PI * i / BALL_POINTS;        
        vector_t *v = malloc(sizeof(*v));
        *v = (vector_t) {BALL_START.x + BALL_RADIUS * cos(angle), 
                         BALL_START.y + BALL_RADIUS * sin(angle)};
        list_add(ball_points, v);
    }
    body_t *ball_bod = body_init(ball_points, BALL_MASS, BALL_COLOR);
    set_ball_velocity(ball_bod);
    scene_add_body(scene, ball_bod);
    create_physics_collision(scene, BALL_ELASTICITY, scene_get_body(scene, 0), ball_bod);
}

// Generates bouncy walls around the top, left, and right perimeter of scene, and creates
// physics collisions between the ball and each of these walls
void bouncy_wall_gen(scene_t *scene){
    body_t *ball = scene_get_body(scene, 1);
    char *c = malloc(1);
    *c = 'W';
    body_t *left_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                 (vector_t) {-MAX_X, MAX_Y / 2}, (rgb_color_t) {1, 1, 1}, c);
    create_physics_collision(scene, BALL_ELASTICITY, left_wall, ball);

    c = malloc(1);
    *c = 'W';

    body_t *right_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                  (vector_t) {2 * MAX_X, MAX_Y / 2}, (rgb_color_t) {1, 1, 1}, c);
    create_physics_collision(scene, BALL_ELASTICITY, right_wall, ball);
    
    c = malloc(1);
    *c = 'W';
    body_t *top_wall = rect_gen(scene, 2 * MAX_X, 2 * MAX_Y, INFINITY, 
                                  (vector_t) {MAX_X / 2, 2 * MAX_Y}, (rgb_color_t) {1, 1, 1}, c);
    create_physics_collision(scene, BALL_ELASTICITY, top_wall, ball);
}

// Checks if the player is over the edge and if so, moves them back to right next to the edge
void check_player_edge(scene_t *scene) {
    body_t *PLAYER = scene_get_body(scene, 0);
    if (body_get_centroid(PLAYER).x - PLAYER_WIDTH / 2 < MIN_X) {
        body_set_centroid(PLAYER, (vector_t) {MIN_X + PLAYER_WIDTH / 2, PLAYER_START.y});
    }
    else if (body_get_centroid(PLAYER).x + PLAYER_WIDTH / 2 > MAX_X) {
        body_set_centroid(PLAYER, (vector_t) {MAX_X - PLAYER_WIDTH / 2, PLAYER_START.y});
    }   
}

// Generates lives as rectangles in the bottom left corner and adds them to scene
void lives_gen(scene_t *scene) {
    for (int i = 0; i < LIVES_AT_START; i ++) {
        double x_loc = LIVES_STARTING_CENTER_X + i * (LIVES_WIDTH + LIVES_HORIZONTAL_SPACE);
        char *c = malloc(1);
        *c = 'L';
        rect_gen(scene, LIVES_WIDTH, LIVES_HEIGHT, INFINITY, (vector_t) {x_loc, LIVES_CENTER_Y}, LIVES_COLOR, c);
    }
}

// Handles key events
void on_key(char key, key_event_type_t type, void *scene) {
    body_t *PLAYER = scene_get_body((scene_t *) scene, 0);
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                body_set_velocity(PLAYER, (vector_t) {-PLAYER_SPEED, 0});
                break;
            case RIGHT_ARROW:
                body_set_velocity(PLAYER, (vector_t) {PLAYER_SPEED, 0});
                break;
        }
    }
    if (type == KEY_RELEASED){
        switch (key) {
            case LEFT_ARROW:
                body_set_velocity(PLAYER, (vector_t) {0,0});
                break;
            case RIGHT_ARROW:
                body_set_velocity(PLAYER, (vector_t) {0,0});
                break;
        }
    }
}

// Returns true if the ball is below the bottom of scene, false if not
bool ball_below_bottom(scene_t *scene) {
    body_t *ball = scene_get_body(scene, 1);
    double centroid = body_get_centroid(ball).y;
    return centroid - BALL_RADIUS < MIN_Y;
}

// Creates and returns a scene that is set up for brickbreaker.
scene_t *setup_game(void) {
    scene_t *scene = scene_init();

    char *c = malloc(1);
    *c = 'P';
    rect_gen(scene, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_MASS, PLAYER_START, PLAYER_COLOR, c);
    ball_gen(scene);
    bouncy_wall_gen(scene);
    brick_gen(scene);
    lives_gen(scene);

    return scene;
}

int main(int argc, char *argv[]) {
    sdl_init((vector_t) {MIN_X, MIN_Y}, (vector_t) {MAX_X, MAX_Y});
    scene_t *scene = setup_game();

    double dt = 0;
    size_t lives = LIVES_AT_START;

    while (!sdl_is_done(scene)) {
        // once bricks are gone or player has lost
        if (scene_bodies(scene) == 5 + lives || lives < 1) {
            break;
        }

        dt = time_since_last_tick();
        sdl_on_key((key_handler_t) on_key);

        if (ball_below_bottom(scene)) {
            lives--;
            scene_remove_body(scene, scene_bodies(scene) - 1);

            body_set_centroid(scene_get_body(scene, 1), BALL_START);
            set_ball_velocity(scene_get_body(scene, 1));
            body_set_centroid(scene_get_body(scene, 0), PLAYER_START);
        }

        scene_tick(scene, dt);
        check_player_edge(scene);
        sdl_render_scene(scene);
    }
    scene_free(scene);
}
