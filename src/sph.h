#ifndef SPH_H
#define SPH_H

#include "cascade.h"
#include "spatial_hash.h"

void sph_compute_density(SimState* sim, SpatialHash* sh);
void sph_compute_forces(SimState* sim, SpatialHash* sh);
void sph_integrate(SimState* sim, float dt);

#endif
