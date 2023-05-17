#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const size_t RESIZE_MULTUPLE = 2;

typedef struct resizable {
  size_t size;
  size_t capacity;
  void **vd;
} resizable_t;

resizable_t *resizable_init(size_t initial_capacity) {
  resizable_t *list = malloc(sizeof(resizable_t));
  assert(list != NULL);
  list->size = 0;
  list->capacity = initial_capacity;
  list->vd = (void **)malloc(initial_capacity * sizeof(void *));
  assert(list->vd != NULL);
  return list;
}

size_t resizable_size(resizable_t *list) { return list->size; }

void resizable_free(resizable_t *list) {
  for (size_t i = 0; i < list->size; i++) {
    free(list->vd[i]);
  }
  free(list->vd);
  free(list);
}

void *resizable_get(resizable_t *list, size_t index) {
  assert(index < list->size);
  return list->vd[index];
}

void resizable_add(resizable_t *list, void *value) {
  assert(value != NULL);
  if (list->size >= list->capacity) {
    list->capacity *= RESIZE_MULTUPLE;
    void **new_data = malloc(list->capacity * sizeof(void *));
    for (size_t i = 0; i < list->size; i++) {
      new_data[i] = list->vd[i];
    }
    list->vd = new_data;
  }
  list->vd[list->size] = value;
  list->size++;
}

void *resizable_remove(resizable_t *list) {
  assert(list->size > 0);
  list->size--;
  return list->vd[list->size];
}

void *resizable_remove_index(resizable_t *list, size_t index) {
  assert(index < list->size);
  assert(list->size > 0);
  void *ret = list->vd[index];
  for (size_t i = index; i < list->size - 1; i++) {
    list->vd[index] = list->vd[index + 1];
  }
  list->size--;
  return ret;
}