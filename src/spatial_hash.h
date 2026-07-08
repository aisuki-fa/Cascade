#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include "cascade.h"

#define GRID_CELLS (256*256)
#define CELL_SIZE 30.0f

typedef struct {
    int cell_count[GRID_CELLS];
    int cell_start[GRID_CELLS];
    int sorted_ids[MAX_PARTICLES];
    int grid_w, grid_h;
    float cell_size;
} SpatialHash;

void sh_build(SpatialHash* sh, Particle* p, int count);
void sh_query(SpatialHash* sh, float x, float y, int* out_ids, int* out_count, int max_out);

#endif
