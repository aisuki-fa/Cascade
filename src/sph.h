#ifndef SPH_H_INCLUDED
#define SPH_H_INCLUDED
#include "cascade.h"
#include "spatial_hash.h"
// ─────────────────────────────────────────────────────────
// sph.h
//
// Declares the 3 SPH simulation step functions, run in order
// each frame after sh_build():
//   1. sph_compute_density  — weighted density sum per particle
//   2. sph_compute_forces   — pressure + viscosity + gravity
//   3. sph_integrate        — apply forces, move particles, bounce
// ─────────────────────────────────────────────────────────
void sph_compute_density(SimState* sim, SpatialHash* sh);
void sph_compute_forces(SimState* sim, SpatialHash* sh);
void sph_integrate(SimState* sim, float dt);
#endif // SPH_H_INCLUDED