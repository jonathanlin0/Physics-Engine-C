#include "body.h"
#include "color.h"
#include "forces.h"
#include "test_util.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

rgb_color_t BLACK = (rgb_color_t){.r = 0.0, .g = 0.0, .b = 0.0};

double vec_dist_calc(vector_t v1, vector_t v2) {
  return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

/**
 * This function makes a list of vectors that is a square.
 */
list_t *make_shape() {
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, +1};
  list_add(shape, v);
  return shape;
}

double gravity_potential(double G, body_t *body1, body_t *body2) {
  vector_t r = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  return -G * body_get_mass(body1) * body_get_mass(body2) / sqrt(vec_dot(r, r));
}

/**
 * calculate the gravity force between 2 bodies
 */
double gravity_force(double G, body_t *body1, body_t *body2) {
  vector_t v1 = body_get_centroid(body1);
  vector_t v2 = body_get_centroid(body2);
  double dist =
      sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
  return G * body_get_mass(body1) * body_get_mass(body2) / pow(dist, 2);
}

/**
 * test to ensure gravity is applying right amount of force for each dt
 */
void f_ma_dt() {
  const double MASS_1 = 5.0, MASS_2 = 10.0;
  const double GRAVITY = 100;
  const double DT = 1e-6;
  const int STEPS = 100000;
  scene_t *scene = scene_init();
  body_t *body_1 = body_init(make_shape(), MASS_1, BLACK);
  scene_add_body(scene, body_1);
  body_t *body_2 = body_init(make_shape(), MASS_2, BLACK);
  body_set_centroid(body_2, (vector_t){20, 0});
  scene_add_body(scene, body_2);
  create_newtonian_gravity(scene, GRAVITY, body_1, body_2);
  for (size_t i = 0; i < STEPS; i++) {
    vector_t last_velocity = body_get_velocity(body_1);
    assert(body_get_centroid(body_1).x < body_get_centroid(body_2).x);
    scene_tick(scene, DT);
    vector_t current_velocity = body_get_velocity(body_1);
    double acceleration = (current_velocity.x - last_velocity.x) / DT;

    double force_from_gravity = gravity_force(GRAVITY, body_1, body_2);
    double ma = MASS_1 * (acceleration);

    assert(within(1e-4, ma, force_from_gravity));
  }
  scene_free(scene);
}

/**
 * test to ensure the distance between the two objects is correct
 */
void distance_test() {
  const double MASS_1 = 5.0, MASS_2 = 10.0;
  const double GRAVITY = 100;
  const double DT = 1e-6;
  const int STEPS = 100000;
  scene_t *scene = scene_init();
  body_t *body_1 = body_init(make_shape(), MASS_1, BLACK);
  scene_add_body(scene, body_1);
  body_t *body_2 = body_init(make_shape(), MASS_2, BLACK);
  body_set_centroid(body_2, (vector_t){20, 0});
  scene_add_body(scene, body_2);
  create_newtonian_gravity(scene, GRAVITY, body_1, body_2);

  vector_t r_initial =
      vec_subtract(body_get_centroid(body_2), body_get_centroid(body_1));
  double last_distance = sqrt(vec_dot(r_initial, r_initial));

  for (size_t i = 0; i < STEPS; i++) {
    vector_t r =
        vec_subtract(body_get_centroid(body_2), body_get_centroid(body_1));
    double distance = sqrt(vec_dot(r, r));

    assert(body_get_centroid(body_1).x < body_get_centroid(body_2).x);

    double acc_1 = gravity_force(GRAVITY, body_1, body_2) / MASS_1;
    double acc_2 = gravity_force(GRAVITY, body_1, body_2) / MASS_2;
    double delta_x_1 =
        (body_get_velocity(body_1).x * DT) + ((1 / 2) * acc_1 * (DT) * (DT));
    double delta_x_2 =
        (body_get_velocity(body_2).x * DT) + ((1 / 2) * acc_2 * (DT) * (DT));

    double expected_distance = last_distance - delta_x_1 - delta_x_2;

    assert(within(1e-3, distance, expected_distance));
    last_distance = distance;

    scene_tick(scene, DT);
  }
  scene_free(scene);
}

/**
 * tests if distance between an anchor and a mass connected by spring force is
 * accurate we know x(t) = A cos(sqrt(K / M) * t), so: v(t) = - A * sqrt(K / M)
 * * sin(sqrt(K / M) * t )
 */
void springs() {
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();
  body_t *mass = body_init(make_shape(), M, BLACK);
  body_set_centroid(mass, (vector_t){A, 0});
  scene_add_body(scene, mass);
  body_t *anchor = body_init(make_shape(), INFINITY, BLACK);
  scene_add_body(scene, anchor);
  create_spring(scene, K, mass, anchor);
  for (int i = 0; i < STEPS; i++) {
    vector_t expected_velocity = (vector_t){
        .x = -1 * A * sqrt(K / M) * sin(sqrt(K / M) * (DT * i)), .y = 0};
    assert(vec_isclose(body_get_velocity(mass), expected_velocity));
    assert(vec_equal(body_get_centroid(anchor), VEC_ZERO));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(f_ma_dt);
  DO_TEST(distance_test);
  DO_TEST(springs);

  puts("student_test PASS");
}