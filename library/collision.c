#include "collision.h"
#include "body.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

bool find_collision(list_t *shape1, list_t *shape2) {
  // check perpendicular axes of first shape
  for (size_t i = 0; i < list_size(shape1); i++) {
    vector_t point1 = *(vector_t *)list_get(shape1, i);
    vector_t point2 =
        *(vector_t *)list_get(shape1, (i + 1) % list_size(shape1));
    vector_t segment =
        (vector_t){.x = point2.x - point1.x, .y = point2.y - point1.y};
    // find unit vector
    double length = vec_l2norm(segment, VEC_ZERO);
    vector_t perpendicular_axis =
        vec_multiply(1 / length, vec_rotate(segment, 3.14 / 2));
    // find projections
    double shape1min = 100000;
    double shape1max = -100000;
    for (size_t j = 0; j < list_size(shape1); j++) {
      vector_t point = *(vector_t *)list_get(shape1, j);
      double projection = vec_dot(point, perpendicular_axis);
      if (projection < shape1min) {
        shape1min = projection;
      }
      if (projection > shape1max) {
        shape1max = projection;
      }
    }
    double shape2min = 100000;
    double shape2max = -100000;
    for (size_t j = 0; j < list_size(shape2); j++) {
      vector_t point = *(vector_t *)list_get(shape2, j);
      double projection = vec_dot(point, perpendicular_axis);
      if (projection < shape2min) {
        shape2min = projection;
      }
      if (projection > shape2max) {
        shape2max = projection;
      }
    }
    // check for no intersection
    if (shape1min > shape2max || shape2min > shape1max) {
      return false;
    }
  }

  // check perpendicular axes of second shape
  for (size_t i = 0; i < list_size(shape2); i++) {
    vector_t point1 = *(vector_t *)list_get(shape2, i);
    vector_t point2 =
        *(vector_t *)list_get(shape2, (i + 1) % list_size(shape2));
    vector_t segment =
        (vector_t){.x = point2.x - point1.x, .y = point2.y - point1.y};
    // find unit vector
    double length = vec_l2norm(segment, VEC_ZERO);
    vector_t perpendicular_axis =
        vec_multiply(1 / length, vec_rotate(segment, 3.14 / 2));
    // find projections
    double shape1min = 100000;
    double shape1max = -100000;
    for (size_t j = 0; j < list_size(shape1); j++) {
      vector_t point = *(vector_t *)list_get(shape1, j);
      double projection = vec_dot(point, perpendicular_axis);
      if (projection < shape1min) {
        shape1min = projection;
      }
      if (projection > shape1max) {
        shape1max = projection;
      }
    }
    double shape2min = 100000;
    double shape2max = -100000;
    for (size_t j = 0; j < list_size(shape2); j++) {
      vector_t point = *(vector_t *)list_get(shape2, j);
      double projection = vec_dot(point, perpendicular_axis);
      if (projection < shape2min) {
        shape2min = projection;
      }
      if (projection > shape2max) {
        shape2max = projection;
      }
    }
    // check for no intersection
    if (shape1min > shape2max || shape2min > shape1max) {
      return false;
    }
  }
  return true;
}
