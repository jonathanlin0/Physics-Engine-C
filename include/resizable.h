#ifndef __RESIZABLE_H__
#define __RESIZABLE_H__

#include "vector.h"
#include <stddef.h>

typedef struct resizable resizable_t;

resizable_t *resizable_init(size_t initial_capacity);

void *resizable_remove_index(resizable_t *list, size_t index);

void resizable_free(resizable_t *list);

size_t resizable_size(resizable_t *list);

void *resizable_get(resizable_t *list, size_t index);

void resizable_add(resizable_t *list, void *value);

void *resizable_remove(resizable_t *list);

#endif // #ifndef __POLYGON_H__