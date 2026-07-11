#ifndef RENDERER_H
#define RENDERER_H

#include "cascade.h"
#include "spatial_hash.h"

void render_blended(SimState* sim,SpatialHash* sh );
void render_solid(SimState* sim);
void render_velocity(SimState* sim);
void render_density(SimState* sim);
void render_particles(SimState* sim, SpatialHash* sh, UIState* ui);
void render_hud(int count, int fps, bool paused);

#endif
