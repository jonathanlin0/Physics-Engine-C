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

// for the body auxillary data, the following rules apply:
// 0 = enemy body
// 1 = player body
// 2 = enemy bullet
// 3 = player bullet

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};
const double EDGE_MARGIN_FOR_ERROR = 3;

// enemy constants
const double ENEMY_RADIUS = 40;
const double ENEMY_ANGLE_RAD = 5 * M_PI / 6;
const size_t ENEMY_LINE_SEGMENTS = 10;
const size_t NUM_ENEMIES_PER_ROW = 8;
const size_t NUM_ENEMIES_ROWS = 3;
const double ENEMY_VERTICAL_MARGIN = 10;
const double ENEMY_HORZ_VELOCITY = 50;
const vector_t INITIAL_ENEMY_LASER_VELOCITY = (vector_t){.x = 0, .y = -100};
const rgb_color_t ENEMY_COLOR = (rgb_color_t){.r = 0, .g = 0, .b = 0};
const rgb_color_t ENEMY_LASER_COLOR = (rgb_color_t){.r = 0, .g = 1, .b = 0};
const double ENEMY_SHOOT_INTERVAL = 0.5;

// player constants (player shape will be an elipse)
const double PLAYER_MINOR_AXIS = 15;
const double PLAYER_MAJOR_AXIS = 40;
const size_t PLAYER_LINE_SEGMENTS = 20;
const vector_t INITIAL_PLAYER_LASER_VELOCITY = (vector_t){.x = 0, .y = 500};
const rgb_color_t PLAYER_COLOR = (rgb_color_t){.b = 0, .g = 0, .r = 1};
const rgb_color_t PLAYER_LASER_COLOR = (rgb_color_t){.b = 1, .g = 1, .r = 0};
const double PLAYER_HORZ_VELOCITY = 1000;
const double PLAYER_BOTTOM_MARGIN = 30;

const double LASER_WIDTH = 1;
const double LASER_HEIGHT = 7;

typedef struct state {
  scene_t *scene;
  size_t enemy_horz_margin;
  double time_elapsed;
} state_t;

// create enemy body according to spawn location
body_t *create_enemy_body(vector_t spawn_loc) {
  list_t *enemy_points = list_init(ENEMY_LINE_SEGMENTS + 1, NULL);
  vector_t *temp = malloc(sizeof(vector_t));
  temp->x = spawn_loc.x;
  temp->y = spawn_loc.y;
  list_add(enemy_points, temp);
  double angle_increment = ENEMY_ANGLE_RAD / ENEMY_LINE_SEGMENTS;
  double start_angle = M_PI / 2 - ENEMY_ANGLE_RAD / 2 + angle_increment / 2;
  for (size_t i = 0; i < ENEMY_LINE_SEGMENTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    point->x =
        (ENEMY_RADIUS * cos(start_angle + (i * angle_increment))) + spawn_loc.x;
    point->y =
        (ENEMY_RADIUS * sin(start_angle + (i * angle_increment))) + spawn_loc.y;
    list_add(enemy_points, point);
  }
  size_t *body_info = malloc(sizeof(size_t));
  *body_info = 0;
  body_t *ret =
      body_init_with_info(enemy_points, 1, ENEMY_COLOR, body_info, free);
  // body_set_centroid(ret, body_get_centroid(ret));
  return ret;
}

// create spawn location in the bottom middle of the screen
body_t *create_player_body() {
  list_t *player_points = list_init(PLAYER_LINE_SEGMENTS, NULL);
  double angle_increment = 2 * M_PI / PLAYER_LINE_SEGMENTS;
  for (size_t i = 0; i < PLAYER_LINE_SEGMENTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    ;
    point->x = PLAYER_MAJOR_AXIS * cos(angle_increment * i);
    point->y = PLAYER_MINOR_AXIS * sin(angle_increment * i);
    list_add(player_points, point);
  }
  size_t *body_info = malloc(sizeof(size_t));
  *body_info = 1;
  body_t *ret =
      body_init_with_info(player_points, 1, PLAYER_COLOR, body_info, free);
  body_set_centroid(ret, (vector_t){CENTER.x, PLAYER_BOTTOM_MARGIN});
  return ret;
}

// create laser body
body_t *create_laser_body(bool is_enemy, vector_t spawn_loc) {
  // laser is just a rectangle
  list_t *laser_points = list_init(4, NULL);
  vector_t *point_1 = malloc(sizeof(vector_t));
  point_1->x = LASER_WIDTH;
  point_1->y = LASER_HEIGHT;
  vector_t *point_2 = malloc(sizeof(vector_t));
  point_2->x = -LASER_WIDTH;
  point_2->y = LASER_HEIGHT;
  vector_t *point_3 = malloc(sizeof(vector_t));
  point_3->x = -LASER_WIDTH;
  point_3->y = -LASER_HEIGHT;
  vector_t *point_4 = malloc(sizeof(vector_t));
  point_4->x = LASER_WIDTH;
  point_4->y = -LASER_HEIGHT;
  list_add(laser_points, point_1);
  list_add(laser_points, point_2);
  list_add(laser_points, point_3);
  list_add(laser_points, point_4);
  if (is_enemy) {
    size_t *body_info = malloc(sizeof(size_t));
    *body_info = 2;
    body_t *ret = body_init_with_info(laser_points, 1, ENEMY_LASER_COLOR,
                                      body_info, free);
    body_set_velocity(ret, INITIAL_ENEMY_LASER_VELOCITY);
    body_set_centroid(ret, spawn_loc);
    return ret;
  } else {
    size_t *body_info = malloc(sizeof(size_t));
    *body_info = 3;
    body_t *ret = body_init_with_info(laser_points, 1, PLAYER_LASER_COLOR,
                                      body_info, free);
    body_set_velocity(ret, INITIAL_PLAYER_LASER_VELOCITY);
    body_set_centroid(ret, spawn_loc);
    return ret;
  }
}

body_t *get_player_body(state_t *state) {
  for (size_t i = 0; i < list_size(scene_get_all_bodies(state->scene)); i++) {
    if (*(size_t *)body_get_info(
            list_get(scene_get_all_bodies(state->scene), i)) == 1) {
      return (body_t *)list_get(scene_get_all_bodies(state->scene), i);
    }
  }
}

void update_player(state_t *state, char key, double held_time) {
  vector_t new_velocity = (vector_t){.x = 0.0, .y = 0.0};
  switch (key) {
  case LEFT_ARROW:
    new_velocity = (vector_t){.x = -1 * PLAYER_HORZ_VELOCITY, .y = 0};
    body_set_velocity(get_player_body(state), new_velocity);
    break;
  case RIGHT_ARROW:
    new_velocity = (vector_t){.x = PLAYER_HORZ_VELOCITY, .y = 0};
    body_set_velocity(get_player_body(state), new_velocity);
    break;
  case UP_ARROW:
    // spawn player bullet after finding the index of player body
    for (size_t i = 0; i < list_size(scene_get_all_bodies(state->scene)); i++) {
      if (*(size_t *)body_get_info(
              list_get(scene_get_all_bodies(state->scene), i)) == 1) {
        body_t *laser =
            create_laser_body(false, body_get_centroid(get_player_body(state)));
        scene_add_body(state->scene, laser);
      }
    }
    break;
  }
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  if (type == KEY_PRESSED) {
    update_player(state, key, held_time);
  }
}

state_t *emscripten_init() {
  srand(time(NULL));
  sdl_on_key(on_key);
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  state->time_elapsed = 0;

  double body_width = ENEMY_RADIUS * 2;
  list_t *enemies =
      list_init(NUM_ENEMIES_PER_ROW * NUM_ENEMIES_ROWS, body_free);

  // checks to ensure that the given enemy width and number of enemies can fit
  // width wise on the screen
  double remaining_width = WINDOW.x;
  remaining_width -= (NUM_ENEMIES_PER_ROW * ENEMY_RADIUS * 2);
  assert(remaining_width > 0);

  // checks that you can fit all the rows of enemies on the top 75% of the
  // screen. leave 25% of the screen at the bottom to fit the player
  double remaining_height = WINDOW.y * 0.75;
  remaining_height -= (NUM_ENEMIES_ROWS * ENEMY_RADIUS);
  if (NUM_ENEMIES_ROWS > 0) {
    remaining_height -= (ENEMY_VERTICAL_MARGIN * (NUM_ENEMIES_PER_ROW - 1));
  }
  assert(remaining_height >= 0);

  // calculate the margin between each enemy horizontally
  double enemy_h_margin = WINDOW.x - (NUM_ENEMIES_PER_ROW * body_width);
  enemy_h_margin /= (NUM_ENEMIES_PER_ROW + 1);
  state->enemy_horz_margin = enemy_h_margin;

  double enemy_pos_y = WINDOW.y - (ENEMY_VERTICAL_MARGIN + ENEMY_RADIUS);
  // create list of enemy bodies
  for (size_t row = 0; row < NUM_ENEMIES_ROWS; row++) {
    double enemy_pos_x = enemy_h_margin + ENEMY_RADIUS;
    for (size_t col = 0; col < NUM_ENEMIES_PER_ROW; col++) {
      vector_t spawn_location = (vector_t){.x = enemy_pos_x, .y = enemy_pos_y};
      body_t *new_enemy = create_enemy_body(spawn_location);
      body_set_velocity(new_enemy,
                        (vector_t){.x = ENEMY_HORZ_VELOCITY, .y = 0});
      list_add(enemies, new_enemy);
      enemy_pos_x += (ENEMY_RADIUS + enemy_h_margin + ENEMY_RADIUS);
    }
    enemy_pos_y -= (ENEMY_VERTICAL_MARGIN + ENEMY_RADIUS);
  }

  // create the player
  body_t *player = create_player_body();

  // add enemies to the scene
  for (size_t i = 0; i < list_size(enemies); i++) {
    scene_add_body(state->scene, list_get(enemies, i));
  }

  // add player to the scene
  scene_add_body(state->scene, player);

  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time_elapsed += dt;
  list_t *enemies =
      list_init(NUM_ENEMIES_PER_ROW * NUM_ENEMIES_ROWS, body_free);

  // get list of enemies
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *curr_body = scene_get_body(state->scene, i);
    if (*(size_t *)(body_get_info(curr_body)) == 0) {
      list_add(enemies, curr_body);
    }
  }

  // get player body
  body_t *player_body;
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *curr_body = scene_get_body(state->scene, i);
    if (*(size_t *)(body_get_info(curr_body)) == 1) {
      player_body = curr_body;
    }
  }

  // draw all bodies
  list_t *all_bodies = scene_get_all_bodies(state->scene);
  for (size_t i = 0; i < list_size(all_bodies); i++) {
    sdl_draw_polygon(body_get_shape(list_get(all_bodies, i)),
                     body_get_color(list_get(all_bodies, i)));
  }

  // GAME OVER SENARIOS--------

  // (1) bullet hits player
  size_t idx = 0;
  while (idx < list_size(all_bodies)) {
    body_t *curr_body = (body_t *)list_get(all_bodies, idx);
    if (*(size_t *)body_get_info(curr_body) == 2) {
      body_t *temp_bullet = (body_t *)list_get(all_bodies, idx);
      aux_t *aux = aux_init(temp_bullet, player_body, 0);
      collision(aux);
      if (body_is_removed(curr_body) == true) {
        exit(0);
      }
    }
    idx++;
  }

  // (2) no more enemies
  if (list_size(enemies) == 0) {
    exit(0);
  }

  // (3) the space invaders fall "below" the ground
  for (size_t i = 0; i < list_size(enemies); i++) {
    // iterate through all the vertices of the shape and check if the y
    // component is below 0
    body_t *curr_enemy = list_get(enemies, i);
    list_t *curr_shape = body_get_shape(curr_enemy);
    for (size_t j = 0; j < list_size(curr_shape); j++) {
      if ((*(vector_t *)(list_get(curr_shape, j))).y < 0) {
        exit(0);
      }
    }
  }
  // -------

  // enemies shoot
  if (state->time_elapsed > ENEMY_SHOOT_INTERVAL) {
    size_t enemy_to_shoot = rand() % list_size(enemies);
    scene_add_body(state->scene,
                   create_laser_body(true, body_get_centroid(list_get(
                                               enemies, enemy_to_shoot))));
    state->time_elapsed -= ENEMY_SHOOT_INTERVAL;
  }

  double left_boundary = state->enemy_horz_margin + ENEMY_RADIUS;
  double right_boundary = WINDOW.x - state->enemy_horz_margin - ENEMY_RADIUS;
  double vertical_diff = -3 * (ENEMY_RADIUS + ENEMY_VERTICAL_MARGIN);
  // move enemy down if necessary
  for (size_t i = 0; i < list_size(enemies); i++) {
    body_t *curr_enemy = list_get(enemies, i);
    // check left boundary
    if (body_get_centroid(curr_enemy).x + 1 < left_boundary) {
      double diff_x = left_boundary - body_get_centroid(curr_enemy).x;
      diff_x += EDGE_MARGIN_FOR_ERROR;
      body_translate(curr_enemy, (vector_t){.x = diff_x, .y = vertical_diff});
      body_set_velocity(curr_enemy,
                        (vector_t){.x = ENEMY_HORZ_VELOCITY, .y = 0});
    }
    // check right boundary
    if (body_get_centroid(curr_enemy).x - 1 > right_boundary) {
      double diff_x = body_get_centroid(curr_enemy).x - right_boundary;
      diff_x += EDGE_MARGIN_FOR_ERROR;
      body_translate(curr_enemy, (vector_t){.x = -diff_x, .y = vertical_diff});
      body_set_velocity(curr_enemy,
                        (vector_t){.x = -ENEMY_HORZ_VELOCITY, .y = 0});
    }
  }

  // delete lasers when they leave the screen
  // enemy bullet is aux value of 2, player is 3
  idx = 0;
  while (idx < list_size(all_bodies)) {
    body_t *curr_body = (body_t *)list_get(all_bodies, idx);
    if (*(size_t *)body_get_info(curr_body) == 2 ||
        *(size_t *)body_get_info(curr_body) == 3) {
      bool need_to_remove = false;
      vector_t body_centroid = body_get_centroid(curr_body);
      // check left and right border
      if (body_centroid.x + (LASER_WIDTH / 2) < 0 ||
          body_centroid.x - (LASER_WIDTH / 2) > WINDOW.x) {
        need_to_remove = true;
      }
      // check top and bottom perimeter
      if (body_centroid.y - (LASER_HEIGHT / 2) > WINDOW.y ||
          body_centroid.y + (LASER_HEIGHT / 2) < 0) {
        need_to_remove = true;
      }
      // remove body if necessary
      if (need_to_remove) {
        body_remove(curr_body);
      }
    }
    idx++;
  }

  scene_tick(state->scene, dt);

  // check collisions
  size_t index = 0;
  while (index < list_size(all_bodies)) {
    for (size_t i = index + 1; i < list_size(all_bodies); i++) {
      body_t *body1 = (body_t *)list_get(all_bodies, index);
      body_t *body2 = (body_t *)list_get(all_bodies, i);
      if (*(size_t *)body_get_info(body1) == 0 &&
          *(size_t *)body_get_info(body2) == 3) {
        // idk if theres a better way than setting the constant to 0
        aux_t *aux = aux_init(body1, body2, 0);
        collision(aux);
      } else if (*(size_t *)body_get_info(body1) == 1 &&
                 *(size_t *)body_get_info(body2) == 2) {
        aux_t *aux = aux_init(body1, body2, 0);
        collision(aux);
      }
    }
    index++;
  }

  // set player velocity back to zero in case arrows aren't pressed
  body_set_velocity(get_player_body(state), VEC_ZERO);
  // set player position to be within the screen
  if (body_get_centroid(get_player_body(state)).x < 0) {
    body_set_centroid(get_player_body(state),
                      (vector_t){0, PLAYER_BOTTOM_MARGIN});
  }
  if (body_get_centroid(get_player_body(state)).x > WINDOW.x) {
    body_set_centroid(get_player_body(state),
                      (vector_t){WINDOW.x, PLAYER_BOTTOM_MARGIN});
  }

  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
