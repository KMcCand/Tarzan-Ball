#include "vector.h"
#include "list.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>

int GROWTH_FACTOR = 2;

typedef struct list {
    void **items;
    size_t size;
    size_t capacity;
    free_func_t free_func;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
    list_t *list = malloc(sizeof(list_t));
    assert(list != NULL && "Could not allocate memory for a new list_t.");
    assert(list->items != NULL && "Could not allocate memory for array of generic types.");
    list->free_func = freer;
    list->size = 0;
    if (initial_size == 0) {
        list->capacity = 1;
    }
    else{
        list->capacity = initial_size;
    }
    list->items = malloc(list->capacity * sizeof(void *));

    return list;
}

void list_free(list_t *list) {
    for (size_t i = 0; i < list->size; i++) {
        if(list->free_func != NULL){
            list->free_func(list->items[i]);
        }
    }
    if(list->size != 0){
        free(list->items);
    }
    free(list);
}

size_t list_size(list_t *list) {
    return list->size;
}

void *list_get(list_t *list, size_t index) {
    assert(index < list->size);
    return list->items[index];
}

void list_add(list_t *list, void *value){
    list_add_back(list, value);
}

int list_index_of(list_t *list, void *value) {
    for (size_t i = 0; i < list->size; i++) {
        if (list->items[i] == value) {
            return i;
        }
    }
    return -1;
}

void list_add_front(list_t *list, void *item) {
    assert(item != NULL);
    if (list->size >= list->capacity) {
        list->items = realloc(list->items, sizeof(void *) * list->size * GROWTH_FACTOR);
        list->capacity *= GROWTH_FACTOR;
    }

    for (int i = list->size - 1; i > -1; i--) {
        list->items[i+1] = list->items[i];
    }
    list->items[0] = item;
    list->size++;
}

void list_add_back(list_t *list, void *item) {
    assert(item != NULL);
    if (list->size >= list->capacity) {
        list->items = realloc(list->items, sizeof(void *) * list->size * GROWTH_FACTOR);
        list->capacity *= GROWTH_FACTOR;
    }
    list->items[list->size] = item;
    list->size++;
}

void *list_remove_back(list_t *list){
    return list_remove(list, list->size);
}

void *list_remove(list_t *list, size_t index) {
    assert(list->size > index > 0);
    void *save = list->items[index];
    for (int i = index; i < list->size - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->size--;
    return save;
}
