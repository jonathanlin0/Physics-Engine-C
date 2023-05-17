#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// aux is an auxillary function
typedef struct body {
  double mass;
  vector_t velocity;
  // old velocity is needed to pass tests,
  // since we should take the average
  vector_t old_velocity;
  vector_t centroid;
  list_t *shape;
  size_t x;
  size_t y;
  rgb_color_t color;
  double orientation;
  double rotational_velocity;
  vector_t net_force;
  bool has_meta_data;
  void *meta_data;
  free_func_t meta_data_freer;
  bool to_be_removed;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, (free_func_t)NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  assert(mass >= 0.0);
  body_t *body = malloc(sizeof(body_t));
  assert(body != NULL);

  body->mass = mass;
  body->velocity = (vector_t){.x = 0.0, .y = 0.0};
  body->old_velocity = (vector_t){.x = 0.0, .y = 0.0};
  body->shape = shape;
  body->centroid = polygon_centroid(shape);
  body->color = (rgb_color_t){.r = color.r, .g = color.g, .b = color.b};
  body->x = 0.0;
  body->y = 0.0;
  body->rotational_velocity = 0.0;
  body->orientation = 0.0;
  body->net_force = (vector_t){.x = 0.0, .y = 0.0};
  body->has_meta_data = false;
  if (info != NULL) {
    body->has_meta_data = true;
  }
  body->meta_data = info;
  body->meta_data_freer = info_freer;
  body->to_be_removed = false;
  return body;
}

void body_free(body_t *body) {
  list_free(body->shape);
  if (body->meta_data_freer != NULL) {
    body->meta_data_freer(body->meta_data);
  }
  free(body);
}

double body_get_mass(body_t *body) { return body->mass; }

// create "deep copy"
list_t *body_get_shape(body_t *body) {
  list_t *out = list_init(1, free);
  list_t *og_shape = body->shape;
  for (size_t i = 0; i < list_size(og_shape); i++) {
    vector_t *new_vec = malloc(sizeof(vector_t));
    new_vec->x = (*((vector_t *)(list_get(og_shape, i)))).x;
    new_vec->y = (*((vector_t *)(list_get(og_shape, i)))).y;
    list_add(out, new_vec);
  }
  return out;
}

double body_area(list_t *shape) {
  double area = 0;

  size_t size = list_size(shape);
  // trapezoid formula
  for (size_t i = 0; i < size; i++) {
    area += (((vector_t *)list_get(shape, i))->y +
             ((vector_t *)list_get(shape, (i + 1) % size))->y) *
            (((vector_t *)list_get(shape, i))->x -
             ((vector_t *)list_get(shape, (i + 1) % size))->x);
  }
  return area / 2;
}

vector_t body_get_centroid(body_t *body) {
  vector_t centroid = (vector_t){.x = 0.0, .y = 0.0};
  double area = body_area(body->shape);
  size_t size = list_size(body->shape);
  for (size_t i = 0; i < size; i++) {
    vector_t vec1 = (*(vector_t *)list_get(body->shape, i));
    vector_t vec2 = (*(vector_t *)list_get(body->shape, (i + 1) % size));
    centroid.x += (vec1.x + vec2.x) * vec_cross(vec1, vec2);
    centroid.y += (vec1.y + vec2.y) * vec_cross(vec1, vec2);
  }
  assert(area != 0);
  return vec_multiply(1 / (6 * area), centroid);
}

vector_t body_get_velocity(body_t *body) { return body->velocity; }

rgb_color_t body_get_color(body_t *body) {
  // why does this commented out code lead to a memory leak??

  // rgb_color_t *ret = malloc(sizeof(rgb_color_t));
  // ret->r = body->color.r;
  // ret->g = body->color.g;
  // ret->b = body->color.b;
  rgb_color_t ret = {
      .r = body->color.r, .g = body->color.g, .b = body->color.b};
  return ret;
}

void *body_get_info(body_t *body) {
  if (body->has_meta_data == true) {
    return body->meta_data;
  }
  return NULL;
}

void body_set_centroid(body_t *body, vector_t x) {
  vector_t difference = vec_subtract(x, body->centroid);
  // translate sets the centroid
  body_translate(body, difference);
}

void body_set_velocity(body_t *body, vector_t v) {
  body->velocity = (vector_t){.x = v.x, .y = v.y};
  body->old_velocity = (vector_t){.x = v.x, .y = v.y};
}

void body_set_rotational_velocity(body_t *body, double value) {
  body->rotational_velocity = value;
}

// different from rotate
void body_set_rotation(body_t *body, double angle) {
  double angle_difference = angle - body->orientation;
  body->orientation = angle;
  vector_t point = body_get_centroid(body);
  size_t size = list_size(body->shape);
  for (size_t i = 0; i < size; i++) {
    vector_t relative_to_point =
        vec_subtract(*(vector_t *)list_get(body->shape, i), point);
    vector_t rotated = vec_rotate(relative_to_point, angle_difference);
    vector_t ret = vec_add(rotated, point);
    (*(vector_t *)list_get(body->shape, i)) = ret;
  }
}
// moves body at its current velocity over a given time interval
void body_tick(body_t *body, double dt) {
  // may have to do average velocity
  vector_t acceleration = vec_multiply(dt / body->mass, body->net_force);
  body->velocity = vec_add(body->velocity, acceleration);
  body_translate(
      body, vec_multiply(dt / 2, vec_add(body->velocity, body->old_velocity)));
  body_set_rotation(body, body->orientation + body->rotational_velocity);
  body->net_force = (vector_t){0, 0};
  body->old_velocity = body->velocity;
}

void body_translate(body_t *body, vector_t translation) {
  size_t size = list_size(body->shape);
  for (size_t i = 0; i < size; i++) {
    *((vector_t *)list_get(body->shape, i)) =
        vec_add(*((vector_t *)list_get(body->shape, i)), translation);
  }
  body->centroid = body_get_centroid(body);
}

vector_t body_get_force(body_t *body) { return body->net_force; }

void body_set_force(body_t *body, vector_t new_force) {
  body->net_force = new_force;
}

void body_add_force(body_t *body, vector_t force) {
  body->net_force = vec_add(body->net_force, force);
}

void body_remove(body_t *body) { body->to_be_removed = true; }

bool body_is_removed(body_t *body) { return body->to_be_removed; }

void body_add_impulse(body_t *body, vector_t impulse) {
  // delta v = impulse / mass
  body->velocity =
      vec_add(body->velocity, vec_multiply(1 / body->mass, impulse));
}