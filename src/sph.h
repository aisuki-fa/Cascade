#ifndef SPH_H_INCLUDED
#define SPH_H_INCLUDED

#include "cascade.h"
#include "spatial_hash.h"

void sph_compute_density(SimState* sim, SpatialHash* sh);
void sph_compute_forces(SimState* sim, SpatialHash* sh);
void sph_integrate(SimState* sim, float dt);

#endif
