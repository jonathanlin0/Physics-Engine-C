#include "list.h"
#include "resizable.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct star {
  list_t *polygon;
  vector_t velocity;
  double rotation;
  double elasticity;
  float COLOR_R;
  float COLOR_G;
  float COLOR_B;
} star_t;

list_t *star_get_polygon(star_t *star) { return star->polygon; }

star_t *make_star(vector_t window, size_t inner_radius, size_t outer_radius,
                  vector_t initial_velocity, size_t num_points) {
  double curr_angle = 0;
  // we'll have n inner radius points and n outer radius points
  double vert_angle = (2.0 * 3.14) / (num_points * 2);
  double x;
  double y;
  list_t *vertices = list_init(num_points * 2, NULL);
  assert(vertices != NULL);
  const vector_t INITIAL_POSITION =
      (vector_t){.x = outer_radius, .y = 500 - outer_radius};

  for (size_t i = 0; i < num_points * 2; i++) {
    double radius = 0;
    if (i % 2 == 0) {
      radius = inner_radius;
    } else {
      radius = outer_radius;
    }
    x = cos(curr_angle) * radius + INITIAL_POSITION.x;
    y = sin(curr_angle) * radius + INITIAL_POSITION.y;
    vector_t *vec_ptr = malloc(sizeof(vector_t));
    vec_ptr->x = x;
    vec_ptr->y = y;
    list_add(vertices, vec_ptr);
    curr_angle += vert_angle;
  }

  star_t *out = malloc(sizeof(star_t));
  out->polygon = vertices;
  out->velocity = (vector_t){.x = initial_velocity.x, .y = 0};

  out->rotation = 1.0;
  out->elasticity = 0.9;

  out->COLOR_R = (double)(rand() % (101)) / 100.0;
  out->COLOR_G = (double)(rand() % (101)) / 100.0;
  out->COLOR_B = (double)(rand() % (101)) / 100.0;

  return out;
}

star_t *make_force_star(vector_t window, size_t inner_radius,
                        size_t outer_radius, vector_t initial_velocity,
                        size_t num_points) {
  double curr_angle = 0;
  // we'll have n inner radius points and n outer radius points
  double vert_angle = (2.0 * 3.14) / (num_points * 2);
  double x;
  double y;
  list_t *vertices = list_init(num_points * 2, NULL);
  assert(vertices != NULL);
  const vector_t INITIAL_POSITION =
      // random star start position
      (vector_t){.x = ((window.x / 2) - 100) + (rand() % 200),
                 .y = ((window.y / 2) - 100) + (rand() % 200)};
  // scale stars to different sizes
  size_t scale = rand();
  for (size_t i = 0; i < num_points * 2; i++) {
    double radius = 0;
    if (i % 2 == 0) {
      radius = scale % inner_radius;
    } else {
      radius = scale % outer_radius;
    }
    x = cos(curr_angle) * radius + INITIAL_POSITION.x;
    y = sin(curr_angle) * radius + INITIAL_POSITION.y;
    vector_t *vec_ptr = malloc(sizeof(vector_t));
    vec_ptr->x = x;
    vec_ptr->y = y;
    list_add(vertices, vec_ptr);
    curr_angle += vert_angle;
  }

  star_t *out = malloc(sizeof(star_t));
  out->polygon = vertices;
  out->velocity = (vector_t){.x = initial_velocity.x, .y = initial_velocity.y};

  out->rotation = 0.0;
  out->elasticity = 0.0;

  // out->COLOR_R = (double)(rand() % (255));
  // out->COLOR_G = (double)(rand() % (255));
  // out->COLOR_B = (double)(rand() % (255));

  return out;
}