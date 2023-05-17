#include "forces.h"
#include "body.h"
#include "collision.h"
#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double MIN_DISTANCE = 5.0;
const double MARGIN = 0.5;

typedef struct aux {
  // two bodies on which the force will act
  body_t *body1;
  body_t *body2;
  // the constant of that force (could be gravity, spring, or drag)
  double force_constant;
} aux_t;

// aux constructor
aux_t *aux_init(body_t *body1, body_t *body2, double constant) {
  aux_t *ret = malloc(sizeof(aux_t));
  assert(ret != NULL);
  ret->body1 = body1;
  ret->body2 = body2;
  ret->force_constant = constant;
  return ret;
}

// aux freer
void free_aux(aux_t *aux) { free(aux); }

void gravity(void *aux) {
  // unwrap data
  body_t *body1 = (body_t *)(((aux_t *)aux)->body1);
  body_t *body2 = (body_t *)(((aux_t *)aux)->body2);
  double G = ((aux_t *)aux)->force_constant;

  // calculate gravity
  vector_t centroid1 = body_get_centroid(body1);
  vector_t centroid2 = body_get_centroid(body2);
  double distance = vec_l2norm(centroid1, centroid2);
  if (distance < MIN_DISTANCE) {
    return;
  }
  double m1 = body_get_mass(body1);
  double m2 = body_get_mass(body2);
  double force = m1 * m2 * G / pow(distance, 2);

  vector_t direction = vec_subtract(centroid2, centroid1);
  // the force vector is the unit direction vector multiplied by the magnitude
  vector_t force_vector = vec_multiply(force / distance, direction);
  body_add_force(body1, force_vector);
  body_add_force(body2, vec_negate(force_vector));
}

void create_newtonian_gravity(scene_t *scene, double gravity_constant,
                              body_t *body1, body_t *body2) {
  // aux_t *aux =  aux_init(body1, body2, gravity_constant);
  aux_t *aux = malloc(sizeof(aux_t));
  aux->body1 = body1;
  aux->body2 = body2;
  aux->force_constant = gravity_constant;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)gravity, aux, bodies,
                                 (free_func_t)free_aux);
}

void spring(void *aux) {
  // unwrap data
  body_t *body1 = (body_t *)(((aux_t *)aux)->body1);
  body_t *body2 = (body_t *)(((aux_t *)aux)->body2);
  double K = ((aux_t *)aux)->force_constant;

  // calculate spring force
  vector_t centroid1 = body_get_centroid(body1);
  vector_t centroid2 = body_get_centroid(body2);

  // direction is the difference between the two vectors
  vector_t direction = vec_subtract(centroid2, centroid1);
  // the force vector is the unit direction vector multiplied by the magnitude
  vector_t force_vector = vec_multiply(K, direction);
  body_add_force(body1, force_vector);
  body_add_force(body2, vec_negate(force_vector));
}

void create_spring(scene_t *scene, double spring_constant, body_t *body1,
                   body_t *body2) {
  // aux_t *aux = aux_init(body1, body2, spring_constant);
  aux_t *aux = malloc(sizeof(aux_t));
  aux->body1 = body1;
  aux->body2 = body2;
  aux->force_constant = spring_constant;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)spring, aux, bodies,
                                 (free_func_t)free_aux);
}

void drag(void *aux) {
  // unwrap data
  aux_t *data = (aux_t *)aux;
  body_t *body = (body_t *)data->body1;
  double D = data->force_constant;

  // calculate drag force
  vector_t force_vector = vec_multiply(D, vec_negate(body_get_velocity(body)));
  body_add_force(body, force_vector);
}

void create_drag(scene_t *scene, double drag_constant, body_t *body) {
  // aux_t *aux = aux_init(body, NULL, drag_constant);
  aux_t *aux = malloc(sizeof(aux_t));
  aux->body1 = body;
  aux->body2 = NULL;
  aux->force_constant = drag_constant;
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, (force_creator_t)drag, aux, bodies,
                                 (free_func_t)free_aux);
}

void collision(void *aux) {
  list_t *shape_1 = body_get_shape((body_t *)(((aux_t *)aux)->body1));
  list_t *shape_2 = body_get_shape((body_t *)(((aux_t *)aux)->body2));
  if (find_collision(shape_1, shape_2)) {
    // set bodies to be removed
    body_remove((body_t *)(((aux_t *)aux)->body1));
    body_remove((body_t *)(((aux_t *)aux)->body2));
  }
  // have to free these lists cuz of the new list created when using
  // body_get_shape()
  list_free(shape_1);
  list_free(shape_2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->body1 = body1;
  aux->body2 = body2;
  aux->force_constant = 0;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)collision, aux, bodies,
                                 (free_func_t)free_aux);
}