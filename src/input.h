#ifndef INPUT_H
#define INPUT_H

#include "cascade.h"

void input_update(SimState* sim, UIState* ui, Vector2 mouse, float dt);
void input_add_particle(SimState* sim, Vector2 pos, Vector2 vel, Color color);

#endif
