#ifndef __PELLET_H__
#define __PELLET_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct pellet pellet_t;

/**
 * Creates a pellet.
 */
pellet_t *make_pellet(vector_t initial_pos);

/**
 * Returns the vector of the center of the pellet
 */
vector_t *position(pellet_t *input_pellet);

void free_pellet(pellet_t *pellet, size_t index);

list_t *get_pellet_polygon(pellet_t *input);
vector_t get_pellet_position(pellet_t *input);
rgb_color_t get_pellet_color(pellet_t *input);

#endif // #ifndef __PELLET_H__