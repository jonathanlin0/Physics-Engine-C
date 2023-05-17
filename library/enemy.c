#include "enemy.h"
#include "body.h"
#include "list.c"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

body_t *make_enemy(vector_t initial_pos, rgb_color_t *color, size_t side_cnt,
                   double mass, double empty_space, double radius) {
  // because the space invaders have a fan shape to them, we will be creating
  // a polygon of 50 sides where the filled areas are 30 ~ 150
  double angle_increment = (2.0 * M_PI) / (side_cnt);
  double curr_angle = 0.0;

  list_t *list_of_points = list_init(2, NULL);
  vector_t *first_vector = malloc(sizeof(vector_t));
  first_vector->x = initial_pos.x;
  first_vector->y = initial_pos.y;
  list_add(list_of_points, first_vector);
  for (size_t i = 0; i < side_cnt; i++) {
    if (curr_angle >= (empty_space) / 2 &&
        curr_angle <= (2 * M_PI) - (radius / 2)) {
      double x = cos(curr_angle) * radius + initial_pos.x;
      double y = sin(curr_angle) * radius + initial_pos.y;

      vector_t *new_vector = malloc(sizeof(vector_t));
      new_vector->x = x;
      new_vector->y = y;
      list_add(list_of_points, new_vector);
    }
    curr_angle += angle_increment;
  }
  return body_init_with_info(list_of_points, ENEMY_MASS, ENEMY_COLOR, 1, NULL);
}

// would the laser beams also use body_init_with_info()?

body_t *enemy_pellet(vector_t current_pos, vector_t constant_velcoity,
                     size_t side_cnt, double mass, double radius) {
  double angle_increment = (2.0 * M_PI) / (side_cnt);
  double curr_angle = 0.0;

  list_t *list_of_points = list_init(2, NULL);
  vector_t *first_vector = malloc(sizeof(vector_t));
  first_vector->x = current_pos.x;
  first_vector->y = current_pos.y;
  list_add(list_of_points, first_vector);

  double x = cos(curr_angle) * radius + current_pos.x;
  double y = sin(curr_angle) * radius + current_pos.y;

  vector_t *new_vector = malloc(sizeof(vector_t));
  new_vector->x = x;
  new_vector->y = y;
  list_add(list_of_points, new_vector);

  curr_angle += angle_increment;

  return body_init_with_info(list_of_points, mass, ENEMY_COLOR);
}

vector_t get_enemy_velocity(body_t *enemy) { return body_get_velocity(enemy); }

vector_t get_enemy_position(body_t *enemy) { return body_get_centroid(enemy); }

void set_enemy_velocity(body_t *enemy, vector_t new_velocity) {
  body_set_velocity(enemy, new_velocity);
}

void set_enemy_position(body_t *enemy, vector_t new_position) {
  body_set_centroid(enemy, new_position);
}
