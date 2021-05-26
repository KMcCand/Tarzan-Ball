#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct textbox {
    int x;
    int y;
    int width;
    int height;
    char *text;
    TTF_Font *font;
    SDL_Color color;
} textbox_t;

textbox_t *textbox_init(int x, int y, int width, int height, char *text, TTF_Font *font, SDL_Color color) {
    textbox_t *new_textbox = malloc(sizeof(textbox_t));
    new_textbox->x = x;
    new_textbox->y = y;
    new_textbox->width = width;
    new_textbox->height = height;
    new_textbox->text = text;
    new_textbox->width = width;
    new_textbox->font = font;
    new_textbox->color = color;
    return new_textbox;
}

int textbox_get_x(textbox_t *textbox) {
    return textbox->x;
}

int textbox_get_y(textbox_t *textbox) {
    return textbox->y;
}

int textbox_get_width(textbox_t *textbox) {
    return textbox->width;
}

int textbox_get_height(textbox_t *textbox) {
    return textbox->height;
}

char *textbox_get_text(textbox_t *textbox) {
    return textbox->text;
}

TTF_Font *textbox_get_font(textbox_t *textbox) {
    return textbox->font;
}

SDL_Color textbox_get_color(textbox_t *textbox) {
    return textbox->color;
}

void textbox_free(textbox_t *textbox) {
    free(textbox->text);
    free(textbox->font);
    free(textbox);
}

