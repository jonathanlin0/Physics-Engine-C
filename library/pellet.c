#include "pellet.h"
#include "body.h"
#include "list.h"
#include "polygon.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const double RADIUS = 5.0;

typedef struct pellet {
  list_t *polygon;
  rgb_color_t color;
} pellet_t;

list_t *get_pellet_polygon(pellet_t *input) { return input->polygon; }
rgb_color_t get_pellet_color(pellet_t *input) { return input->color; }

pellet_t *make_pellet(vector_t initial_pos) {
  pellet_t *out = malloc(sizeof(pellet_t));

  // create a pellet with 10 edges to represent a circle
  double angle_incremenet = (2.0 * 3.14) / (10);
  double curr_angle = 0.0;

  list_t *list_of_points = list_init(10, NULL);
  assert(list_of_points != NULL);
  double x;
  double y;

  for (size_t i = 0; i < 10; i++) {
    x = cos(curr_angle) * RADIUS + initial_pos.x;
    y = sin(curr_angle) * RADIUS + initial_pos.y;

    // vector_t new_vector = (vector_t){.x = x, .y = y};
    // list_add(list_of_points, &new_vector);
    vector_t *new_vector = malloc(sizeof(vector_t));
    new_vector->x = x;
    new_vector->y = y;
    list_add(list_of_points, new_vector);

    curr_angle += angle_incremenet;
  }

  out->polygon = list_of_points;
  out->color = (rgb_color_t){.r = 1.0, .g = 0.0, .b = 0.0};

  return out;
}

void free_pellet(pellet_t *pellet, size_t index) {
  list_free(pellet->polygon);
  free(&(pellet->color.r));
  free(&(pellet->color.g));
  free(&(pellet->color.b));
  free(pellet);
}