#ifndef RENDERER_H
#define RENDERER_H

#include "cascade.h"
#include "spatial_hash.h"

void render_particles(SimState* sim, SpatialHash* sh, UIState* ui);
void render_hud(int count, int fps, bool paused);

#endif
