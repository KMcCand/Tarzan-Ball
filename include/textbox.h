#ifndef __TEXTBOX_H__
#define __TEXTBOX_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct textbox textbox_t;

textbox_t *textbox_init(int x, int y, int width, int height, char *text, TTF_Font *font, SDL_Color color);

int textbox_get_x(textbox_t *textbox);

int textbox_get_y(textbox_t *textbox);

int textbox_get_width(textbox_t *textbox);

int textbox_get_height(textbox_t *textbox);

char *textbox_get_text(textbox_t *textbox);

TTF_Font *textbox_get_font(textbox_t *textbox);

SDL_Color textbox_get_color(textbox_t *textbox);

void textbox_free(textbox_t *textbox);

#endif
