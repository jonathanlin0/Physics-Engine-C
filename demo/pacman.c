#include "body.h"
#include "list.h"
#include "pellet.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// pacman constants
const rgb_color_t PACMAN_COLOR = (rgb_color_t){.r = 0.5, .g = 1.0, .b = 0.0};
const double PACMAN_RADIUS = 30;
const double PACMAN_ACCELERATION = 1;
const size_t NUM_PACMAN_SIDES = 48;
const size_t PACMAN_MASS = 2.0;
const vector_t PACMAN_SPAWN_LOCATION = (vector_t){.x = 200, .y = 200};
const size_t INITIAL_DT = 10.0;

// angle of entire mouth
const double PACMAN_MOUTH_ANGLE_RADIANS = M_PI / 3;

// game scene constants
const double PELLET_SPAWN_INTERVAL = 1;

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

body_t *create_pacman(vector_t initial_pos) {
  // we are creating packman using a polygon of 48 sides.
  // should be enough to "look" like a circle
  // the mouth is +- 30 degrees, so there will be 42 points making up the
  // "circle" part of the packman, and then a vector in the middle so 43 points
  double angle_incremenet = (2.0 * M_PI) / (NUM_PACMAN_SIDES);
  double curr_angle = 0.0;

  list_t *list_of_points = list_init(2, NULL);
  vector_t *first_vector = malloc(sizeof(vector_t));
  first_vector->x = initial_pos.x;
  first_vector->y = initial_pos.y;
  list_add(list_of_points, first_vector);

  for (size_t i = 0; i < NUM_PACMAN_SIDES; i++) {
    if (curr_angle >= (PACMAN_MOUTH_ANGLE_RADIANS) / 2 &&
        curr_angle <= (2 * M_PI) - (PACMAN_MOUTH_ANGLE_RADIANS / 2)) {

      double x = cos(curr_angle) * PACMAN_RADIUS + initial_pos.x;
      double y = sin(curr_angle) * PACMAN_RADIUS + initial_pos.y;

      vector_t *new_vector = malloc(sizeof(vector_t));
      new_vector->x = x;
      new_vector->y = y;
      list_add(list_of_points, new_vector);
    }
    curr_angle += angle_incremenet;
  }

  rgb_color_t *new_pacman_color = malloc(sizeof(rgb_color_t));
  new_pacman_color->r = PACMAN_COLOR.r;
  new_pacman_color->g = PACMAN_COLOR.g;
  new_pacman_color->b = PACMAN_COLOR.b;

  return body_init(list_of_points, PACMAN_MASS, *new_pacman_color);
}

typedef struct state {
  list_t *pellets;
  body_t *pacman;
  double time_elapsed;
  size_t start_index;
  double dt;
} state_t;

void loop_around(body_t *pacman) {
  if (body_get_centroid(pacman).x > WINDOW.x) {
    body_translate(pacman, (vector_t){-WINDOW.x, 0});
  }
  if (body_get_centroid(pacman).x < 0) {
    body_translate(pacman, (vector_t){WINDOW.x, 0});
  }
  if (body_get_centroid(pacman).y > WINDOW.y) {
    body_translate(pacman, (vector_t){0, -WINDOW.y});
  }
  if (body_get_centroid(pacman).y < 0) {
    body_translate(pacman, (vector_t){0, WINDOW.y});
  }
}

void update_pacman(state_t *state, char key, double held_time) {
  vector_t new_velocity = {0, 0};
  switch (key) {
  case LEFT_ARROW:
    new_velocity = (vector_t){.x = -PACMAN_ACCELERATION * held_time, .y = 0};
    break;
  case RIGHT_ARROW:
    new_velocity = (vector_t){.x = PACMAN_ACCELERATION * held_time, .y = 0};
    break;
  case UP_ARROW:
    new_velocity = (vector_t){.x = 0, .y = PACMAN_ACCELERATION * held_time};
    break;
  case DOWN_ARROW:
    new_velocity = (vector_t){.x = 0, .y = -PACMAN_ACCELERATION * held_time};
    break;
  }
  body_set_velocity(state->pacman, new_velocity);
  if (new_velocity.x > 0) {
    body_set_rotation(state->pacman, 3.14 * 2);
  }
  if (new_velocity.x < 0) {
    body_set_rotation(state->pacman, 3.14);
  }
  if (new_velocity.y < 0) {
    body_set_rotation(state->pacman, 3.14 * 3 / 2);
  }
  if (new_velocity.y > 0) {
    body_set_rotation(state->pacman, 3.14 / 2);
  }

  body_tick(state->pacman, 10);
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  if (type == KEY_PRESSED) {
    update_pacman(state, key, held_time);
  }
}

state_t *emscripten_init() {
  sdl_on_key(on_key);

  srand(time(NULL));
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->pacman = create_pacman(
      (vector_t){.x = PACMAN_SPAWN_LOCATION.x, .y = PACMAN_SPAWN_LOCATION.y});
  body_set_centroid(state->pacman, body_get_centroid(state->pacman));
  state->pellets = list_init(1, NULL);
  state->time_elapsed = 0.0;
  state->start_index = 0;

  return state;
}

// runs each tick of the loop and shows the results
// takes the same state as initialized in emscripten_init
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time_elapsed += dt;
  sdl_is_done(state);

  loop_around(state->pacman);

  // draw pacman
  rgb_color_t not_normalized_color = body_get_color(state->pacman);
  rgb_color_t normalized_color =
      (rgb_color_t){.r = not_normalized_color.r / 255,
                    .g = not_normalized_color.g / 255,
                    .b = not_normalized_color.b / 255};
  sdl_draw_polygon(body_get_shape(state->pacman), normalized_color);

  // spawn a new pellet every x seconds
  if (state->time_elapsed >= PELLET_SPAWN_INTERVAL) {
    // create a pointer to a new pellet
    vector_t rand_pos = (vector_t){.x = (double)(rand() % (size_t)(WINDOW.x)),
                                   .y = (double)(rand() % (size_t)(WINDOW.y))};
    pellet_t *new_pellet = make_pellet(rand_pos);

    list_add(state->pellets, new_pellet);
    state->time_elapsed = state->time_elapsed - PELLET_SPAWN_INTERVAL;
  }

  // redraw pellets
  for (size_t i = 0; i < list_size(state->pellets); i++) {
    pellet_t *curr_pellet = list_get(state->pellets, i);

    for (size_t j = 0; j < list_size(get_pellet_polygon(curr_pellet)); j++) {
      vector_t point =
          *((vector_t *)list_get(get_pellet_polygon(curr_pellet), j));
      double distance = vec_l2norm(point, body_get_centroid(state->pacman));
      if (distance < PACMAN_RADIUS) {
        list_remove(state->pellets, i);
        break;
      }
    }

    sdl_draw_polygon(get_pellet_polygon(curr_pellet),
                     get_pellet_color(curr_pellet));
  }

  sdl_show();
}

// frees the memory associated with everything
void emscripten_free(state_t *state) {
  for (size_t i = 0; i < list_size(state->pellets); i++) {
    free(list_get(state->pellets, i));
  }
  body_free(state->pacman);
  free(state);
}