#include "color.h"
#include "forces.h"
#include "polygon.h"
#include "scene.h"
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

const double rotation = 1;
const double MIN_DISTANCE1 = 20;

// star constants
const size_t NUM_STAR_POINTS = 5;

const double TWO_PI = 2.0 * 3.14;

const size_t NUM_STARS = 25;
const double GRAVITATIONAL_CONSTANT = 1.0e4;
const double MAX_INITAL_VELOCITY = 15;
const double MAX_ROTATIONAL_VELOCITY = 0.01;
const double MAX_MASS = 100;

typedef struct state {
  scene_t *stars;
} state_t;

// get a random position in the window
vector_t get_random_pos(vector_t window_frame) {
  return (vector_t){(float)rand() * window_frame.x / (float)RAND_MAX,
                    (float)rand() * window_frame.y / (float)RAND_MAX};
}

// get a random velocity
vector_t get_random_velocity(double max_initial_vel) {
  return (vector_t){(((float)rand() / (float)RAND_MAX) - 0.5) * max_initial_vel,
                    (((float)rand() / (float)RAND_MAX) - 0.5) *
                        max_initial_vel};
}

// get a random rotational velocity
double get_random_rotational_velocity(double max_initial_vel) {
  return (((float)rand() / (float)RAND_MAX) - 0.5) * max_initial_vel;
}

// get a random color
rgb_color_t *get_random_color() {
  rgb_color_t *out = malloc(sizeof(rgb_color_t));
  out->r = (float)rand() / (float)RAND_MAX;
  out->g = (float)rand() / (float)RAND_MAX;
  out->b = (float)rand() / (float)RAND_MAX;
  return out;
}

// create separate random functions for color position
state_t *emscripten_init() {
  srand(time(NULL));
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->stars = scene_init();
  for (size_t i = 0; i < NUM_STARS; i++) {
    // create random color
    rgb_color_t *color = get_random_color();

    // create random mass (radii are proportional to sqrt of mass)
    double mass = (float)rand() * MAX_MASS / (float)RAND_MAX + 1;
    size_t radius = (size_t)sqrt(mass);
    // create random position
    vector_t random_position = get_random_pos(WINDOW);
    // create random velocity
    vector_t random_velocity = get_random_velocity(MAX_INITAL_VELOCITY);
    // create random rotation
    double random_rotational_velocity =
        get_random_rotational_velocity(MAX_ROTATIONAL_VELOCITY);
    // create a star and use it's polygon to create a body
    star_t *temp_star_obj = make_star(WINDOW, radius, (size_t)(radius * 2),
                                      (vector_t){0, 0}, NUM_STAR_POINTS);
    body_t *star = body_init(star_get_polygon(temp_star_obj), mass, *color);
    body_set_centroid(star, random_position);
    body_set_velocity(star, random_velocity);
    body_set_rotational_velocity(star, random_rotational_velocity);
    scene_add_body(state->stars, star);
  }

  // for every unique pair of bodies, initialize a gravitational force between
  // them
  list_t *bodies = scene_get_all_bodies(state->stars);
  for (size_t i = 0; i < list_size(bodies) - 1; i++) {
    body_t *star1 = list_get(bodies, i);
    for (size_t j = i + 1; j < list_size(bodies); j++) {
      body_t *star2 = list_get(bodies, j);
      create_newtonian_gravity(state->stars, GRAVITATIONAL_CONSTANT, star1,
                               star2);
    }
  }

  return state;
}

void emscripten_main(state_t *state) {

  sdl_clear();
  double dt = time_since_last_tick();
  scene_tick(state->stars, dt);

  // draw the polygons
  list_t *bodies = scene_get_all_bodies(state->stars);
  for (size_t i = 0; i < list_size(bodies); i++) {
    void *body = list_get(bodies, i);
    rgb_color_t color = body_get_color(body);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, color);
  }
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->stars);
  free(state);
}
