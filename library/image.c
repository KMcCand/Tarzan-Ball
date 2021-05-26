#include "image.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "vector.h"

typedef struct image {
    SDL_Surface *surface;
    vector_t dimensions;
    double rotation;
} image_t;

image_t *image_init(char *name, vector_t dimensions, double rotation) {
    image_t *image = malloc(sizeof(image_t));
    image->surface = IMG_Load(name);
    assert(image->surface != NULL && "Could not generate SDL_Surface from image name");
    free(name);
    image->dimensions = dimensions;
    image->rotation = rotation;
    return image;
}

SDL_Surface *image_get_surface(image_t *image) {
    return image->surface;
}

vector_t image_get_dimensions(image_t *image) {
    return image->dimensions;
}

double image_get_rotation(image_t *image) {
    return image->rotation;
}

void image_free(image_t *image) {
    SDL_FreeSurface(image->surface);
    free(image);
}
