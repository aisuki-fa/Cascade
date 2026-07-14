#ifndef INPUT_H                                        // include guard: skip if already defined
#define INPUT_H                                        // mark this header as included

#include "cascade.h"                                   // SimState, UIState, ObstacleList types

// Handles mouse-based particle spawning and keyboard shortcut
void input_update(SimState* sim, UIState* ui, ObstacleList* obs, Vector2 mouse, float dt);
// Adds a single particle to the simulation at given pos/vel/color
void input_add_particle(SimState* sim, Vector2 pos, Vector2 vel, Color color);

#endif                                                 // end of include guard
