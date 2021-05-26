#include "body.h"
#include "list.h"
#include "vector.h"
#include "sdl_wrapper.h"
#include "forces.h"
#include "scene.h"
#include "color.h"
#include "polygon.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Player drawing constants
const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 1000.0;
const double MAX_Y = 1000.0;
const size_t PLR_PTS = 50;
const double PLR_CENTER_X = 500.0;
const double PLR_CENTER_Y = 10.0;
const double PLR_SQUASH = 0.2;
const double PLR_LEN = 50.0;
const double PLR_MASS = 50.0;
const double BULLET_W_HALF = 2.5;
const double BULLET_L_HALF = 4.0;
const double BULLET_MASS = 10.0;
const vector_t BULLET_SPD = (vector_t) {0.0, 400.0};
const rgb_color_t PLR_COLOR = (rgb_color_t) {0.0, 1.0, 0.0};
const double PLAYER_SPEED = 2000.0;

// Enemy ship drawing constants
const double ENEMY_SHIP_RADIUS = 50;
const double ENEMY_SHIP_ANGLE = 2.5;
const size_t ENEMY_SHIP_NPOINTS = 20;
const double ENEMY_SHIP_MASS = 10.0;
const rgb_color_t ENEMY_SHIP_COLOR = {0.1, 0.1, 0.1};
const vector_t ENEMY_STARTING_VELOCITY = {200, 0};

// Enemy spatial organization constants
const double ENEMY_NUMBER_ACROSS = 12;
const double ENEMY_LATERAL_SPACE_FACTOR = 0.2;
const vector_t ENEMY_COL_START_LOC = {40, 920};
const double ENEMY_COL_NUMBER = 3;
const double ENEMY_Y_SPACE = 15;
const size_t ENEMY_NEW_COLS_MAX = 5;
const double ENEMY_Y_COORD_TO_LOSE = 100.0;

// Enemy shooting constants
const double ENEMY_BULLET_TIME_MIN = 0.7;
const double ENEMY_BULLET_TIME_MAX = 1.5;
const double ENEMY_BULLET_TIME_DECREASE_FACTOR = 0.98;
const double ENEMY_SHOOTING_CHANCE = 0.05;


/**
 * Creates one enemy ship body_t with bottom tip at bottom_point and adds it to scene.
 * The enemy ship is made according to the constants titled "Enemy ship drawing constants".
 * 
 * @param scene the place to put the new enemy ship body_t
 * @param bottom_point the location of the bottom point at which to put the enemy ship
 */ 
void create_enemy_body(scene_t *scene, vector_t *bottom_point) {
    list_t *enemy_ship_points = list_init(ENEMY_SHIP_NPOINTS, (free_func_t) vec_free);
    list_add(enemy_ship_points, bottom_point);

    for (size_t i = 0; i < ENEMY_SHIP_NPOINTS - 1; i++) {
        double angle = M_PI / 2 + ENEMY_SHIP_ANGLE / (ENEMY_SHIP_NPOINTS - 1) * ((double) i - (ENEMY_SHIP_NPOINTS - 1) / 2);
        vector_t *one_point = malloc(sizeof(vector_t));
        *one_point = (vector_t) {bottom_point->x + ENEMY_SHIP_RADIUS * cos(angle), bottom_point->y + ENEMY_SHIP_RADIUS * sin(angle)};
        list_add(enemy_ship_points, one_point);
    }

    char *player_or_enemy = malloc(1);
    *player_or_enemy = 'E';
    body_t *enemy_ship_body = body_init_with_info(enemy_ship_points, ENEMY_SHIP_MASS, ENEMY_SHIP_COLOR, player_or_enemy, free);
    body_set_velocity(enemy_ship_body, ENEMY_STARTING_VELOCITY);
    scene_add_body(scene, enemy_ship_body);
}

/**
 * Creates a vertical column of enemy ships, all with their bottom points at x_loc, and
 * adds them to scene. The ships have specs as according to "Enemy ship drawing constants",
 * and are spaced as according to "Enemy spatial organization constants".
 * 
 * @param scene the place to put the new column of enemy ships
 * @param x_loc the x location that all of the ships should have
 */ 
void create_enemy_column(scene_t *scene, double x_loc) {
    for (int i = 0; i < ENEMY_COL_NUMBER; i++) {
        vector_t *bottom_point = malloc(sizeof(vector_t));
        *bottom_point = (vector_t) {x_loc, ENEMY_COL_START_LOC.y - i * (ENEMY_Y_SPACE + ENEMY_SHIP_RADIUS)};
        create_enemy_body(scene, bottom_point);
    }
}

/**
 * Creates the starting grid of enemies and puts them in scene. Spaces a number of enemy columns
 * evenly horizontally and as dictated by the "Enemy spatial organization constants".
 * 
 * @param scene the place to put the grid of enemy ships
 */ 
void create_enemy_grid(scene_t *scene) {
    const double ship_width = (MAX_X - MIN_X) / (ENEMY_NUMBER_ACROSS + ENEMY_LATERAL_SPACE_FACTOR * (ENEMY_NUMBER_ACROSS + 1));
    const double space_between = ENEMY_LATERAL_SPACE_FACTOR* ship_width;

    for (size_t i = 0; i < ENEMY_NUMBER_ACROSS; i++) {
        create_enemy_column(scene, MIN_X + space_between + ship_width / 2 + i * (space_between + ship_width));
    }
}

/**
 * Checks if any enemies have hit any edges of the screen. If they hit the bottom edge, returns -1
 * to indicate the user lost. If they hit the left or right edge, moves them down, reverses their velocity,
 * and if allowed_to_add is true, adds a new enemy column to scene and returns 1, else returns 0.
 * 
 * @param scene the scene on which to check the enemies and edges
 * @param allowed_to_add true if the number of enemy columns added is less than ENEMY_NEW_COLS_MAX, and false otherwise.
 * If true and enemies go off the right or left edge, adds another enemy column at the starting location.
 * @return -1 if the user lost, 1 if a new column of ships was added, and 0 otherwise
 */ 
int check_enemy_edges(scene_t *scene, bool allowed_to_add) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *current_body = scene_get_body(scene, i);
        char type = ((char *) body_get_info(current_body))[0];

        if (type == 'E') {
            list_t *points = body_get_shape(current_body);
            vector_t *left_point = list_get(points, 1);
            vector_t *right_point = list_get(points, ENEMY_SHIP_NPOINTS - 1);
             
            if (((vector_t *) list_get(points, 0))->y <= ENEMY_Y_COORD_TO_LOSE) {
                return -1;
            }
            else if (left_point->x <= MIN_X || right_point->x >= MAX_X) {
                body_set_velocity(current_body, vec_negate(body_get_velocity(current_body)));
                double new_y = body_get_centroid(current_body).y - ENEMY_COL_NUMBER * (ENEMY_Y_SPACE + ENEMY_SHIP_RADIUS);
                body_set_centroid(current_body, (left_point->x <= MIN_X) ? (vector_t) {MIN_X, new_y} : (vector_t) {MAX_X, new_y});

                if (allowed_to_add) {
                    create_enemy_column(scene, ENEMY_COL_START_LOC.x);
                    return 1;
                }
            }
        }
    }

    return 0;
}

/**
 * Helper for create bullet, creates a new collision force between each
 * potential target and the bullet when a bullet is added
 * 
 * @param scene the interactive game environment containing all bodies
 * @param new_body the newly added body to which collisions must accordingly be added
 */ 
void create_new_collisions(scene_t *scene, body_t *new_body){
   for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *curr = scene_get_body(scene, i);
        char curr_type = ((char *) body_get_info(curr))[0];
        char new_body_type = ((char *) body_get_info(new_body))[0];

        if ((curr_type == '1' && new_body_type == 'E') ||
           (curr_type == 'E' && new_body_type == '1') ||
           (curr_type == '2' && new_body_type == 'P') ||
           (curr_type == 'P' && new_body_type == '2')) {
            create_destructive_collision(scene, curr, new_body);
        }
    }
}

/**
 * Creates a bullet from either a player or enemy. Accordingly adds collision
 * forces between bodies that can get hit by the bullet.
 * 
 * @param scene the interactive game environment containing all bodies
 * @param body the player or enemy that will shoot the bullet
 */ 
void create_bullet(scene_t *scene, body_t *body) {
    char* body_type = (char *) body_get_info(body);
    assert(body_type[0] == 'P' || body_type[0] == 'E');
    
    list_t *bullet_pts = list_init(4, (free_func_t) vec_free);
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t) {-BULLET_W_HALF, -BULLET_L_HALF};
    list_add(bullet_pts, v);
    v = malloc(sizeof(*v));
    *v = (vector_t) {BULLET_W_HALF, -BULLET_L_HALF};
    list_add(bullet_pts, v);
    v = malloc(sizeof(*v));
    *v = (vector_t) {BULLET_W_HALF, BULLET_L_HALF};
    list_add(bullet_pts, v);
    v = malloc(sizeof(*v));
    *v = (vector_t) {-BULLET_W_HALF, BULLET_L_HALF};
    list_add(bullet_pts, v);

    char *bullet_info = malloc(1);
    rgb_color_t bullet_color = body_get_color(body);

    if (body_type[0] == 'P') {
        *bullet_info = '1';
        body_t *bullet = body_init_with_info(bullet_pts, BULLET_MASS, bullet_color, bullet_info, free);
        vector_t centroid = (vector_t) {body_get_centroid(body).x, body_get_centroid(body).y + 25.0};
        body_set_centroid(bullet, centroid);
        body_set_velocity(bullet, BULLET_SPD);
        scene_add_body(scene, bullet);
        create_new_collisions(scene, bullet);
    }
    else {
        *bullet_info = '2';
        body_t *bullet = body_init_with_info(bullet_pts, BULLET_MASS, bullet_color, bullet_info, free);
        list_t *enemy_pts = body_get_shape(body);
        vector_t *enemy_tip = (vector_t*) list_get(enemy_pts, 0);
        vector_t centroid = (vector_t) {enemy_tip->x, enemy_tip->y - 10.0};
        body_set_centroid(bullet, centroid);
        body_set_velocity(bullet, vec_multiply(-1, BULLET_SPD));
        scene_add_body(scene, bullet);
        create_new_collisions(scene, bullet);
    }     
}

/**
 * Deletes any bullets that fly off the screen
 * 
 * @param scene the interactive game environment containing all bodies
 */ 
void check_bullets_offscreen(scene_t *scene) {
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *curr_body = scene_get_body(scene, i);
        char curr_info = ((char *) body_get_info(curr_body))[0];
        if (curr_info == '1' || curr_info == '2'){
            if (body_get_centroid(curr_body).y > MAX_Y || body_get_centroid(curr_body).y < MIN_Y) {
                body_remove(scene_get_body(scene, i));
            }
        }
    }
}

/**
 * Returns a random double between min_time and max_time
 */ 
double get_random_bullet_time(double min_time, double max_time) {
    return rand() / RAND_MAX * (max_time - min_time) + min_time;
}

/**
 * Gives each enemy ENEMY_SHOOTING_CHANCE probability of firing a bullet. Bullets are fired
 * downward from the bottom tip of the enemy.
 * 
 * @param scene a scene with enemies in it
 */ 
void enemy_shoot_bullet(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *current_body = scene_get_body(scene, i);
        if (((char *) body_get_info(current_body))[0] == 'E' && (double) rand() / (double) RAND_MAX <= ENEMY_SHOOTING_CHANCE) {
            create_bullet(scene, current_body);
        }
    }
}

/**
 * Creates player oval
 * 
 * @param scene interactive environment for player
 */ 
void create_player_body(scene_t *scene) {
    list_t *plr_pts = list_init(PLR_PTS, (free_func_t) vec_free);

    for(size_t i = 0; i < PLR_PTS; i++){
        double angle = 2 * M_PI * i / PLR_PTS;
        vector_t *v = malloc(sizeof(vector_t));
        *v = (vector_t) {PLR_CENTER_X + PLR_LEN * cos(angle),
                                  PLR_CENTER_Y + PLR_SQUASH * PLR_LEN * sin(angle)};
        list_add(plr_pts, v);
    }

    char *c = malloc(1);
    *c = 'P';
    body_t *plr_bod = body_init_with_info(plr_pts, PLR_MASS, PLR_COLOR, c, free);
    scene_add_body(scene, plr_bod);
}

/**
 * Checks if the player is off the edge of scene. If it is, moves it back
 * to the edge they went off of.
 */ 
void check_player_edge(scene_t *scene) {
    body_t *PLAYER = scene_get_body(scene, 0);
    if (body_get_centroid(PLAYER).x - PLR_LEN < MIN_X) {
        body_set_centroid(PLAYER, (vector_t) {MIN_X + PLR_LEN, PLR_CENTER_Y});
    }
    else if (body_get_centroid(PLAYER).x + PLR_LEN > MAX_X) {
        body_set_centroid(PLAYER, (vector_t) {MAX_X - PLR_LEN, PLR_CENTER_Y});
    }   
}
/**
 * Handles key presses.
 */ 
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
            case ' ':
                create_bullet(scene, PLAYER);
                break;
        }
    }
    if (type == KEY_RELEASED){
        switch (key){
            case LEFT_ARROW:
                body_set_velocity(PLAYER, (vector_t) {0,0});
                break;
            case RIGHT_ARROW:
                body_set_velocity(PLAYER, (vector_t) {0,0});
                break;
        }
    }
}

int main (int argc, char *argv[]) {
    sdl_init((vector_t) {MIN_X, MIN_Y}, (vector_t) {MAX_X, MAX_Y});
    scene_t *scene = scene_init();
    create_player_body(scene);
    create_enemy_grid(scene);

    double dt = 0;
    double bullet_time_min = ENEMY_BULLET_TIME_MIN;
    double bullet_time_max = ENEMY_BULLET_TIME_MAX;
    double time_until_next_bullet = get_random_bullet_time(bullet_time_min, bullet_time_max);
    size_t number_new_enemy_cols = 0;
    int output = 0;

    while (!sdl_is_done(scene)) {
        if (output < 0 || ((char *) body_get_info(scene_get_body(scene, 0)))[0] != 'P') {
            continue;
        }
        dt = time_since_last_tick();
        
        time_until_next_bullet -= dt;
        if (time_until_next_bullet <= 0) {
            enemy_shoot_bullet(scene);
            bullet_time_min *= ENEMY_BULLET_TIME_DECREASE_FACTOR;
            bullet_time_max *= ENEMY_BULLET_TIME_DECREASE_FACTOR;
            time_until_next_bullet = get_random_bullet_time(bullet_time_min, bullet_time_max);
        }
        
        sdl_on_key((key_handler_t) on_key);

        check_bullets_offscreen(scene);
        output = check_enemy_edges(scene, number_new_enemy_cols < ENEMY_NEW_COLS_MAX * ENEMY_COL_NUMBER);
        number_new_enemy_cols += output;

        scene_tick(scene, dt);
        check_player_edge(scene);
        sdl_render_scene(scene);
    }

    scene_free(scene);
}
