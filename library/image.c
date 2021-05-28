#include "image.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct image {
    SDL_Surface *surface;
    vector_t dimensions;
    double rotation;
    bool show;
} image_t;

image_t *image_init(char *name, vector_t dimensions, double rotation) {
    image_t *image = malloc(sizeof(image_t));
    image->surface = IMG_Load(name);
    assert(image->surface != NULL && "Could not generate SDL_Surface from image name");
    image->dimensions = dimensions;
    image->rotation = rotation;
    image->show = true;
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

void image_set_show(image_t *image, bool new_value) {
    image->show = new_value;
}

bool image_get_show(image_t *image) {
    return image->show;
}

void image_free(image_t *image) {
    SDL_FreeSurface(image->surface);
    free(image);
}
