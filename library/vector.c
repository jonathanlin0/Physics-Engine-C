#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

const vector_t VEC_ZERO = (vector_t){.x = 0.0, .y = 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t ret = {.x = v1.x + v2.x, .y = v1.y + v2.y};
  return ret;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t ret = {.x = v1.x - v2.x, .y = v1.y - v2.y};
  return ret;
}

double vec_l2norm(vector_t v1, vector_t v2) {
  return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

vector_t vec_negate(vector_t v) {
  vector_t ret = vec_multiply(-1, v);
  return ret;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t ret = {.x = v.x * scalar, .y = v.y * scalar};
  return ret;
}

double vec_dot(vector_t v1, vector_t v2) {
  double ret = (v1.x * v2.x) + (v1.y * v2.y);
  return ret;
}

double vec_cross(vector_t v1, vector_t v2) {
  double ret = (v1.x * v2.y) - (v1.y * v2.x);
  return ret;
}

vector_t vec_rotate(vector_t v, double angle) {
  vector_t ret = {.x = (cos(angle) * v.x) - (sin(angle) * v.y),
                  .y = (sin(angle) * v.x) + (cos(angle) * v.y)};
  return ret;
}
