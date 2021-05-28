#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stddef.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include "vector.h"

typedef struct image image_t;

/**
* Initializes and returns a pointer to an image_t with an SDL surface created
* with the image at name and the dimensions in dimensions. Frees the memory at name.
* 
* @param name the name of the image
* @param dimensions the dimensions of the image
* @param rotation the rotation of the image
* @return a pointer to an image_t
*/
image_t *image_init(char *name, vector_t dimensions, double rotation);

/**
* Returns a pointer to the SDL_Surface of image 
*/
SDL_Surface *image_get_surface(image_t *image);

/**
* Returns the dimensions of an image
*/
vector_t image_get_dimensions(image_t *image);

/**
* Returns the rotation of an image
*/
double image_get_rotation(image_t *image);

/**
* Sets whether the image will be rendered or not to new_value
*/
void image_set_show(image_t *image, bool new_value);

/**
* Returns whether the image will be rendered or not
*/
bool image_get_show(image_t *image);

/**
* Frees the memory associated with image
*/
void image_free(image_t *image);

#endif // #ifndef __IMAGE_H__
