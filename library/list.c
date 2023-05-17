#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const size_t RESIZE_MULTIPLE = 2;

typedef struct list {
  size_t size;
  size_t capacity;
  void **vd;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer_input) {
  list_t *list = malloc(sizeof(list_t));
  assert(list != NULL);
  list->size = 0;
  // do this because if capacity is initially 0, then the list can't grow in
  // size cuz 0 * RESIZE_MULTIPLE is till 0
  list->capacity = initial_size;
  if (initial_size <= 0) {
    list->capacity = 1;
    initial_size += 1;
  }
  list->vd = (void **)malloc(initial_size * sizeof(void *));
  assert(list->vd != NULL);
  list->freer = freer_input;
  return list;
}

size_t list_size(list_t *list) { return list->size; }

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->vd[i]);
    }
  }
  free(list->vd);
  free(list);
}

void *list_get(list_t *list, size_t index) {
  assert(index < list->size);
  return list->vd[index];
}

void resize(list_t *list) {
  size_t new_capacity = 100 + list->capacity * RESIZE_MULTIPLE;
  list->vd = realloc(list->vd, sizeof(void *) * new_capacity * 2);
  list->capacity = new_capacity;
}

void list_add(list_t *list, void *value) {
  assert(list->capacity != 0);
  if (list->capacity == list->size) {
    resize(list);
  }
  assert(value != NULL);
  assert(list->size < list->capacity);
  list->vd[list->size] = value;
  list->size++;
}

void remover_helper(list_t *list, size_t index) {
  for (size_t i = index; i < list->size - 1; i++) {
    list->vd[i] = list_get(list, i + 1);
  }
  list->vd[list->size - 1] = NULL;
}

void *list_remove(list_t *list, size_t index) {
  assert(index < list->size);
  assert(list->size > 0);
  assert(index >= 0);
  void *ret = list->vd[index];
  remover_helper(list, index);
  list->size--;
  return ret;
}