#include "forces.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "star.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};
const double TWO_PI = 2.0 * 3.14;

const size_t NUM_BALLS = 750;
const size_t NUM_CIRCLE_POINTS = 20;

// ball constants
const size_t BALL_RADIUS = 4;
const double BALL_MASS = 1e3;
const vector_t INITIAL_BALL_VELOCITY = (vector_t){.x = 0, .y = 0};
const size_t ANCHOR_RADIUS = 1;

const double SPRING_CONSTANT = 1e4;
const double DRAG_CONSTANT = 10;

typedef struct state {
  scene_t *balls;
} state_t;

// get a random color
rgb_color_t *get_random_color() {
  rgb_color_t *out = malloc(sizeof(rgb_color_t));
  out->r = (float)rand() / (float)RAND_MAX;
  out->g = (float)rand() / (float)RAND_MAX;
  out->b = (float)rand() / (float)RAND_MAX;
  return out;
}

// get the position vector of the balls in the sin formation
vector_t get_position_sin(double init_pos_x, vector_t screen_center,
                          vector_t window) {
  return (vector_t){init_pos_x,
                    screen_center.y +
                        screen_center.y * cos(init_pos_x * 40 / window.x)};
}

// get the position vector of the balls in the pulse formation
vector_t get_position_pulse(double init_pos_x, vector_t screen_center,
                            vector_t window) {
  vector_t out;
  if (init_pos_x < 40) {
    out = (vector_t){init_pos_x,
                     screen_center.y +
                         screen_center.y * cos(init_pos_x * 10 / window.x)};
  } else {
    out = (vector_t){init_pos_x, screen_center.y};
  }
  return out;
}

state_t *emscripten_init() {
  srand(time(NULL));
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->balls = scene_init();
  for (size_t i = 0; i < NUM_BALLS; i++) {
    // create random color
    rgb_color_t *color = get_random_color();
    double position_x = WINDOW.x * i / NUM_BALLS + BALL_RADIUS;

    // arrange the balls in a sin wave:
    // vector_t position = get_position_sin(position_x, CENTER, WINDOW);

    // arrange the balls to make a pulse:
    vector_t position = get_position_pulse(position_x, CENTER, WINDOW);

    // create a cicle (star with equal inner and outer radii) and use it's
    // polygon to create a body
    star_t *temp_star_obj = make_star(WINDOW, BALL_RADIUS, BALL_RADIUS,
                                      INITIAL_BALL_VELOCITY, NUM_CIRCLE_POINTS);
    body_t *star =
        body_init(star_get_polygon(temp_star_obj), BALL_MASS, *color);
    body_set_centroid(star, position);
    scene_add_body(state->balls, star);
  }

  // for every adjacent pair of balls, initialize a spring force between them
  list_t *bodies = scene_get_all_bodies(state->balls);
  for (size_t i = 0; i < list_size(bodies) - 1; i++) {
    body_t *star1 = list_get(bodies, i);
    body_t *star2 = list_get(bodies, i + 1);
    create_spring(state->balls, SPRING_CONSTANT, star1, star2);
  }

  // create two immovable balls (infinite mass) at both ends to anchor the chain
  rgb_color_t *color = get_random_color();

  star_t *temp_star1 = make_star(WINDOW, ANCHOR_RADIUS, ANCHOR_RADIUS,
                                 INITIAL_BALL_VELOCITY, NUM_CIRCLE_POINTS);
  body_t *left = body_init(star_get_polygon(temp_star1), INFINITY, *color);
  body_set_centroid(left, (vector_t){0, CENTER.y});
  create_spring(state->balls, SPRING_CONSTANT, left,
                list_get(scene_get_all_bodies(state->balls), 0));

  star_t *temp_star2 = make_star(WINDOW, ANCHOR_RADIUS, ANCHOR_RADIUS,
                                 INITIAL_BALL_VELOCITY, NUM_CIRCLE_POINTS);
  body_t *right = body_init(star_get_polygon(temp_star2), INFINITY, *color);
  body_set_centroid(right, (vector_t){WINDOW.x, CENTER.y});
  create_spring(state->balls, SPRING_CONSTANT, right,
                list_get(scene_get_all_bodies(state->balls), NUM_BALLS - 1));

  // for every ball, create a drag force
  for (size_t i = 0; i < list_size(bodies); i++) {
    body_t *star = list_get(bodies, i);
    // drag proportional to x position
    create_drag(state->balls, DRAG_CONSTANT, star);
  }

  return state;
}

void emscripten_main(state_t *state) {

  sdl_clear();
  double dt = time_since_last_tick();
  scene_tick(state->balls, dt);

  // draw the polygons
  list_t *bodies = scene_get_all_bodies(state->balls);
  for (size_t i = 0; i < list_size(bodies); i++) {
    void *body = list_get(bodies, i);
    rgb_color_t color = body_get_color(body);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, color);
  }
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->balls);
  free(state);
}