#ifndef __STAR_H__
#define __STAR_H__

#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct star star_t;

list_t *star_get_polygon(star_t *star);

star_t *make_star(vector_t window, size_t inner_radius, size_t outer_radius,
                  vector_t initial_velocity, size_t num_points);

star_t *make_force_star(vector_t window, size_t inner_radius,
                        size_t outer_radius, vector_t initial_velocity,
                        size_t num_points);

#endif // #ifndef __STAR_H__