#include "scene.h"
#include "body.h"
#include "forces.h"
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const size_t INITIAL_NUM_BODIES = 2;

typedef struct scene {
  list_t *bodies;
  list_t *forces;
} scene_t;

scene_t *scene_init() {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene != NULL);
  scene->bodies = list_init(INITIAL_NUM_BODIES, (free_func_t)body_free);
  scene->forces = list_init(INITIAL_NUM_BODIES, (free_func_t)force_free);
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->forces);
  free(scene);
}

typedef struct force {
  force_creator_t forcer;
  void *aux;
  free_func_t aux_freer;
  list_t *bodies;
} force_t;

force_t *force_init(force_creator_t forcer, void *aux_data, list_t *bodies,
                    free_func_t aux_freer) {
  force_t *force = malloc(sizeof(force_t));
  assert(force != NULL);
  force->forcer = *forcer;
  force->aux = aux_data;
  force->aux_freer = aux_freer;
  force->bodies = bodies;
  return force;
}

void force_free(force_t *forcer) {
  if (forcer->aux_freer != NULL) {
    forcer->aux_freer(forcer->aux);
  }
  if (forcer->bodies != NULL) {
    list_free(forcer->bodies);
  }
  free(forcer);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

size_t scene_forces(scene_t *scene) { return list_size(scene->forces); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return (body_t *)list_get(scene->bodies, index);
}

list_t *scene_get_all_bodies(scene_t *scene) { return scene->bodies; }

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

bool force_is_removed(force_t *force, body_t *rem) {
  list_t *bodies = force->bodies;
  for (size_t i = 0; i < list_size(bodies); i++) {
    if ((body_t *)list_get(bodies, i) == rem) {
      return true;
    }
  }
  return false;
}

// this function removes the necessary bodies from the scene
void remove_bodies(scene_t *scene) {
  list_t *bodies = scene->bodies;
  list_t *forces = scene->forces;
  size_t bodies_idx = 0;
  // using while loop instead of for loop cuz modifying list(s) as iterating
  // through it
  while (bodies_idx < list_size(bodies)) {
    body_t *temp_body = (body_t *)list_get(bodies, bodies_idx);
    if (body_is_removed(temp_body)) {
      size_t forces_idx = 0;
      while (forces_idx < list_size(forces)) {
        if (force_is_removed((force_t *)list_get(forces, forces_idx),
                             temp_body)) {
          force_free(list_remove(forces, forces_idx));
        } else {
          forces_idx += 1;
        }
      }
      body_free(list_remove(bodies, bodies_idx));
    } else {
      bodies_idx += 1;
    }
  }
}

void scene_tick(scene_t *scene, double dt) {
  // apply every force in the forces list
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_t *force = (force_t *)list_get(scene->forces, i);
    force->forcer(force->aux);
  }
  // tick the bodies
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_tick((body_t *)(list_get(scene->bodies, i)), dt);
  }
  // remove the necessary bodies
  remove_bodies(scene);
}

// depreciated. only still exists for backward compatibility
void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}

// the aux data is as struct that holds the 2 bodies involved in the force
void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_t *force = force_init(forcer, aux, bodies, freer);
  list_add(scene->forces, force);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_t *body = list_get(scene->bodies, index);
  body_remove(body);
  // list_remove(scene->bodies, index);
  // body_free(body);
}