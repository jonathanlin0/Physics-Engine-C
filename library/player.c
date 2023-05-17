#include "player.h"
#include "body.h"
#include "list.c"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const rgb_color_t PLAYER_COLOR = (rgb_color_t){.r = 0.2, .g = 1.0, .b = 0.2};

const double INCREASE = 1.02;

const double DECREASE = 0.98;

typedef struct player {
  vector_t position;
  vector_t velocity;
  body_t *body;
} player_t;

body_t *make_player(vector_t initial_pos, vector_t const_velocity,
                    size_t side_cnt, double mass, double curr_radius) {
  // because the space invaders have a fan shape to them, we will be creating
  // a polygon of 100 sides (we want our player to resemble the shape of an
  // oval) in order to draw the oval we can divide it into quadrants of varying
  // radius lengths
  double angle_increment = (2.0 * M_PI) / (side_cnt);
  double curr_angle = 0.0;

  list_t *list_of_points = list_init(2, NULL);
  vector_t *first_vector = malloc(sizeof(vector_t));
  first_vector->x = initial_pos.x;
  first_vector->y = initial_pos.y;
  list_add(list_of_points, first_vector);
  double cmp = (side_cnt / 4);
  double rate = DECREASE;
  for (size_t i = 0; i < side_cnt; i++) {
    double x = cos(curr_angle) * curr_radius + initial_pos.x;
    double y = cos(curr_angle) * curr_radius + initial_pos.y;
    vector_t *new_vector = malloc(sizeof(vector_t));
    new_vector->x = x;
    new_vector->y = y;
    list_add(list_of_points, new_vector);
    curr_angle += angle_increment;
    curr_radius *= rate;
    if ((double)i > cmp) {
      cmp += cmp;
      if (rate == DECREASE) {
        rate = INCREASE;
      } else {
        rate = DECREASE;
      }
    }
  }
  // create a type struct of body_info...
  return body_init_with_info(list_of_points, mass, PLAYER_COLOR, 0.0, NULL);
}