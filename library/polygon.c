#include "polygon.h"
#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {
  double area = 0;

  size_t size = list_size(polygon);
  // trapezoid formula
  for (size_t i = 0; i < size; i++) {
    area += ((((vector_t *)list_get(polygon, i))->y +
              ((vector_t *)list_get(polygon, (i + 1) % size))->y) *
             (((vector_t *)list_get(polygon, i))->x -
              ((vector_t *)list_get(polygon, (i + 1) % size))->x));
  }
  return area / 2;
}

vector_t polygon_centroid(list_t *polygon) {
  vector_t centroid = (vector_t){.x = 0.0, .y = 0.0};
  double area = polygon_area(polygon);

  // centroid formula (mod hangles last point to first point)
  size_t size = list_size(polygon);
  for (size_t i = 0; i < size; i++) {
    vector_t *vec1_pointer = ((vector_t *)(list_get(polygon, i)));
    vector_t *vec2_pointer = ((vector_t *)(list_get(polygon, (i + 1) % size)));
    vector_t vec1 = *vec1_pointer;
    vector_t vec2 = *vec2_pointer;
    centroid.x += (vec1.x + vec2.x) * vec_cross(vec1, vec2);
    centroid.y += (vec1.y + vec2.y) * vec_cross(vec1, vec2);
  }
  return vec_multiply(1 / (6 * area), centroid);
}

void polygon_translate(list_t *polygon, vector_t translation) {
  size_t size = list_size(polygon);
  for (size_t i = 0; i < size; i++) {
    *((vector_t *)list_get(polygon, i)) =
        vec_add(*((vector_t *)list_get(polygon, i)), translation);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  size_t size = list_size(polygon);
  for (size_t i = 0; i < size; i++) {
    vector_t *temp = list_get(polygon, i);
    vector_t relative_to_point = vec_subtract((vector_t)(*temp), point);
    vector_t rotated = vec_rotate(relative_to_point, angle);
    vector_t ret = vec_add(rotated, point);
    *(vector_t *)list_get(polygon, i) = ret;
  }
}