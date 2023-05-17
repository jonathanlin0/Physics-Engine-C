#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

const vector_t INITIAL_VELOCITY = (vector_t){.x = 100, .y = 100};
const double rotation = 1;

// circle constants
const size_t NUM_STAR_POINTS = 5;
const size_t INNER_RAD = 20;
const size_t OUTER_RAD = 200;

// color constants
const float YELLOW_R = 1;
const float YELLOW_G = 1;
const float YELLOW_B = 0.0;

const double TWO_PI = 2.0 * 3.14;

list_t *make_a_star(size_t innerRadius, size_t outerRadius, size_t num_points,
                    size_t center_x, size_t center_y) {
  double curr_angle = 0;
  // we'll have n inner radius points and n outer radius points
  double vert_angle = TWO_PI / (num_points * 2);
  double x;
  double y;
  list_t *vertices = list_init(num_points * 2, NULL);
  assert(vertices != NULL);
  for (size_t i = 0; i < num_points * 2; i++) {
    double radius = 0;
    if (i % 2 == 0) {
      radius = innerRadius;
    } else {
      radius = outerRadius;
    }
    x = cos(curr_angle) * radius + center_x;
    y = sin(curr_angle) * radius + center_y;
    vector_t *vec_ptr = malloc(sizeof(vector_t));
    vec_ptr->x = x;
    vec_ptr->y = y;
    list_add(vertices, vec_ptr);
    curr_angle += vert_angle;
  }
  return vertices;
}

// state definition (required in every demo)
// you can put anything in here!
// ideally only things that can change and are relevant to the main loop
typedef struct state {
  vector_t velocity;
  list_t *star;
} state_t;

// initializes the state to hold the star polygon and returns the state
state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->velocity.x = INITIAL_VELOCITY.x;
  state->velocity.y = INITIAL_VELOCITY.y;
  state->star =
      make_a_star(INNER_RAD, OUTER_RAD, NUM_STAR_POINTS, CENTER.x, CENTER.y);
  return state;
}

// runs each tick of the loop and shows the results
// takes the same state as initialized in emscripten_init
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();

  list_t *star = state->star;
  polygon_rotate(star, dt * rotation, polygon_centroid(star));
  polygon_translate(star, vec_multiply(dt, state->velocity));

  for (size_t i = 0; i < list_size(star); i++) {
    vector_t *point = (vector_t *)(list_get(star, i));
    // places the object the distance it would have travelled away from wall
    if (point->x > WINDOW.x || point->x < 0) {
      vector_t adjustment =
          (vector_t){.x = -2 * fmod(point->x, WINDOW.x), .y = 0};
      polygon_translate(state->star, adjustment);
      state->velocity.x = -state->velocity.x;
    }
    if (point->y > WINDOW.y || point->y < 0) {
      vector_t adjustment =
          (vector_t){.x = 0, .y = -2 * fmod(point->y, WINDOW.y)};
      polygon_translate(state->star, adjustment);
      state->velocity.y = -state->velocity.y;
    }
  }

  // your sdl_draw_polygon will take less arguments
  sdl_draw_polygon(star,
                   (rgb_color_t){.r = YELLOW_R, .g = YELLOW_G, .b = YELLOW_B});
  sdl_show();
}

// frees the memory associated with everything
void emscripten_free(state_t *state) {
  list_t *star = state->star;
  free(star);
  free(state);
}