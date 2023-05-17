#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "body.h"
#include "list.h"
#include "vector.h"
#include "body.h"
#include "color.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

body_t *make_enemy(vector_t initial_pos, rgb_color_t *color, size_t side_cnt,
                   double mass, double empty_space, double radius);

body_t *enemy_pellet(vector_t current_pos, vector_t constant_velocity,
                     size_t side_cnt, double mass, double radius);

vector_t get_enemy_velocity(body_t *enemy);

vector_t get_enemy_position(body_t *enemy);

void set_enemy_velocity(body_t *enemy, vector_t new_velocity);

void set_enemy_position(body_t *enemy, vector_t new_position);

#endif //__ENEMY_H__