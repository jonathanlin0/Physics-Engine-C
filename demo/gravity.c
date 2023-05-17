#include "list.h"
#include "polygon.h"
#include "resizable.h"
#include "sdl_wrapper.h"
#include "star.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct star {
  list_t *polygon;
  vector_t velocity;
  double rotation;
  double elasticity;
  float COLOR_R;
  float COLOR_G;
  float COLOR_B;
} star_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

// circle constants
const size_t NUM_STAR_POINTS = 5;
const size_t INNER_RAD = 20;
const size_t OUTER_RAD = 40;

const double GRAVITY_ACCELERATION = -100.0;
const vector_t INITIAL_VELOCITY = (vector_t){.x = 50, .y = 0};
const double rotation = 1;

const double TWO_PI = 2.0 * 3.14;
const double star_interval = 3.14;

// state definition (required in every demo)
// you can put anything in here!
// ideally only things that can change and are relevant to the main loop
typedef struct state {
  resizable_t *stars;
  double time_elapsed;
  size_t n;
  size_t start_index;
} state_t;

// initializes the state to hold the star polygon and returns the state
state_t *emscripten_init() {
  srand(time(NULL));
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->n = 4;
  state->start_index = 0;
  resizable_t *stars = resizable_init(1);

  // create the first star
  star_t *first_star =
      make_star(WINDOW, INNER_RAD, OUTER_RAD, INITIAL_VELOCITY, state->n);

  // add first star to list of stars
  resizable_add(stars, first_star);
  state->stars = stars;
  state->time_elapsed = 0.0;

  return state;
}

// runs each tick of the loop and shows the results
// takes the same state as initialized in emscripten_init
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time_elapsed += dt;

  if (state->time_elapsed >= star_interval) {
    state->n = state->n + 1;
    star_t *new_star =
        make_star(WINDOW, INNER_RAD, OUTER_RAD, INITIAL_VELOCITY, state->n);
    resizable_add(state->stars, new_star);
    state->time_elapsed = state->time_elapsed - star_interval;
  }

  for (size_t i = state->start_index; i < resizable_size(state->stars); i++) {
    star_t *curr_star = resizable_get(state->stars, i);

    // calculate new y velocity for curr star
    double new_vel = curr_star->velocity.y + (GRAVITY_ACCELERATION * dt);
    curr_star->velocity.y = new_vel;

    polygon_rotate(curr_star->polygon, dt * curr_star->rotation,
                   polygon_centroid(curr_star->polygon));
    polygon_translate(curr_star->polygon,
                      vec_multiply(dt, curr_star->velocity));

    // handle when star hits edge
    for (size_t i = 0; i < list_size(curr_star->polygon); i++) {
      vector_t *point = (vector_t *)list_get(curr_star->polygon, i);
      // places the object the distance it would have travelled away from wall

      if (point->y < 0) {
        vector_t adjustment =
            (vector_t){.x = 0, .y = -2 * fmod(point->y, WINDOW.y)};
        polygon_translate(curr_star->polygon, adjustment);
        curr_star->velocity.y = curr_star->elasticity * -curr_star->velocity.y;
      }
    }
    sdl_draw_polygon(curr_star->polygon,
                     (rgb_color_t){.r = curr_star->COLOR_R,
                                   .g = curr_star->COLOR_G,
                                   .b = curr_star->COLOR_B});
  }

  star_t *first_star = resizable_get(state->stars, state->start_index);
  if (polygon_centroid(first_star->polygon).x - OUTER_RAD > WINDOW.x) {
    state->start_index = state->start_index + 1;
  }

  // your sdl_draw_polygon will take less arguments
  sdl_show();
}

// frees the memory associated with everything
void emscripten_free(state_t *state) {
  for (size_t i = 0; i < resizable_size(state->stars); i++) {
    free(resizable_get(state->stars, i));
  }
  free(state);
}
