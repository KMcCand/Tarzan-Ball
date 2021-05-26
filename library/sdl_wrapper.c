#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "sdl_wrapper.h"
#include "textbox.h"

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
    int *width = malloc(sizeof(*width)),
        *height = malloc(sizeof(*height));
    assert(width != NULL);
    assert(height != NULL);
    SDL_GetWindowSize(window, width, height);
    vector_t dimensions = {.x = *width, .y = *height};
    free(width);
    free(height);
    return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
    // Scale scene so it fits entirely in the window
    double x_scale = window_center.x / max_diff.x,
           y_scale = window_center.y / max_diff.y;
    return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
    // Scale scene coordinates by the scaling factor
    // and map the center of the scene to the center of the window
    vector_t scene_center_offset = vec_subtract(scene_pos, center);
    double scale = get_scene_scale(window_center);
    vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
    vector_t pixel = {
        .x = round(window_center.x + pixel_center_offset.x),
        // Flip y axis since positive y is down on the screen
        .y = round(window_center.y - pixel_center_offset.y)
    };
    return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT:  return LEFT_ARROW;
        case SDLK_UP:    return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN:  return DOWN_ARROW;
        case SDLK_r: return R_KEY;
        case SDLK_s: return S_KEY;
        case SDLK_w: return W_KEY;
        case SDLK_p: return P_KEY;
        case SDLK_d: return D_KEY;
        case SDLK_t: return T_KEY;
        case SDLK_k: return K_KEY;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

void sdl_init(vector_t min, vector_t max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max));
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
}

bool sdl_is_done(void *data) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event != NULL);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                TTF_Quit();
                IMG_Quit();
                free(event);
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (key_handler == NULL) break;

                char key = get_keycode(event->key.keysym.sym);
                if (key == '\0') break;

                uint32_t timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                key_event_type_t type = 
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(key, type, held_time, data, VEC_ZERO);
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (key_handler == NULL) break;
                SDL_ShowCursor(false);
                key_event_type_t type_mouse = event->type == SDL_MOUSEBUTTONDOWN ? KEY_PRESSED : KEY_RELEASED;
                type_mouse = event->type == SDL_MOUSEMOTION ? MOUSE_ENGAGED : type_mouse;

                vector_t loc = (vector_t) {event->motion.x, event->motion.y};
                key = type_mouse == MOUSE_ENGAGED ? MOUSE_MOVED : MOUSE_CLICK;

                held_time = (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(key, type_mouse, held_time, data, loc);
                break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    vector_t window_center = get_window_center();

    // Convert each vertex to a point on screen
    int16_t *x_points = malloc(sizeof(*x_points) * n),
            *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points != NULL);
    assert(y_points != NULL);
    for (size_t i = 0; i < n; i++) {
        vector_t *vertex = list_get(points, i);
        vector_t pixel = get_window_position(*vertex, window_center);
        x_points[i] = pixel.x;
        y_points[i] = pixel.y;
    }

    // Draw polygon with the given color
    filledPolygonRGBA(
        renderer,
        x_points, y_points, n,
        color.r * 255, color.g * 255, color.b * 255, 255
    );
    free(x_points);
    free(y_points);
}

void sdl_show(scene_t *scene, list_t *textboxes) {
    // Draw boundary lines
    vector_t window_center = get_window_center();
    vector_t max = vec_add(center, max_diff),
             min = vec_subtract(center, max_diff);
    vector_t max_pixel = get_window_position(max, window_center),
             min_pixel = get_window_position(min, window_center);
    SDL_Rect *boundary = malloc(sizeof(*boundary));
    boundary->x = min_pixel.x;
    boundary->y = max_pixel.y;
    boundary->w = max_pixel.x - min_pixel.x;
    boundary->h = min_pixel.y - max_pixel.y;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, boundary);

    // Render the background image under everything. Potential optimization: this doesn't have to be done over and over again
    SDL_Texture *background_texture = SDL_CreateTextureFromSurface(renderer, image_get_surface(scene_get_background(scene)));
    assert(background_texture != NULL && "Scene background texture is null!");
    SDL_RenderCopyEx(renderer, background_texture, NULL, boundary, 0, NULL, SDL_FLIP_NONE);
    free(boundary);
    SDL_DestroyTexture(background_texture);
    
    // Render all bodies except for the cursor and the player image and put their images on top of them
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        char *body_info = (char *) body_get_info(body);

        if (((char *) body_info)[0] != 'C' && ((char *) body_info)[0] != 'I') {
            list_t *shape = body_get_shape(body);
            sdl_draw_polygon(shape, body_get_color(body));
            list_free(shape);
            
            if (body_has_image(body) && ! ((char *) body_info)[0] != 'P') {
                image_t *body_image = body_get_image(body);
                vector_t centroid = body_get_centroid(scene_get_body(scene, i));
                vector_t image_dimensions = image_get_dimensions(body_image);

                SDL_Rect *image_bounds = malloc(sizeof(*image_bounds));
                image_bounds->x = centroid.x - image_dimensions.x / 2;
                image_bounds->y = 500 - image_dimensions.y / 2 - centroid.y;
                image_bounds->w = image_dimensions.x;
                image_bounds->h = image_dimensions.y;

                SDL_Texture *image_texture = SDL_CreateTextureFromSurface(renderer, image_get_surface(body_image));
                SDL_RenderCopyEx(renderer, image_texture, NULL, image_bounds, image_get_rotation(body_image), NULL, SDL_FLIP_NONE);
                free(image_bounds);
            }
        }
    }

    // Render the cursor last to make it on top of everything
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        char *body_info = (char *) body_get_info(body);
        if (((char *) body_info)[0] == 'C' || ((char *) body_info)[0] == 'I') {
            list_t *shape = body_get_shape(body);
            sdl_draw_polygon(shape, body_get_color(body));
            list_free(shape);
        }
    }

    sdl_render_text(scene, textboxes);

    SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene, list_t *textboxes) {
    sdl_clear();
    sdl_show(scene, textboxes);
}

void sdl_render_text(scene_t *scene, list_t *textboxes){
    for(size_t i = 0; i < list_size(textboxes); i++) {
        textbox_t *tb= (textbox_t *) list_get(textboxes, i);
        SDL_Surface* surfaceMessage = TTF_RenderText_Blended_Wrapped(textbox_get_font(tb), textbox_get_text(tb),
                                                                     textbox_get_color(tb), textbox_get_width(tb)/2); 
        SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect rect; //create a rect
        rect.x = textbox_get_x(tb);  //controls the rect's x coordinate 
        rect.y = textbox_get_y(tb);  // controls the rect's y coordinte
        rect.w = textbox_get_width(tb);  // controls the width of the rect
        rect.h = textbox_get_height(tb);  // controls the height of the rect
        SDL_RenderCopy(renderer, Message, NULL, &rect);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(Message);
    }
}

void sdl_on_key(key_handler_t handler) {
    key_handler = handler;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}

void sdl_free() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
