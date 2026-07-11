#include "input.h"

void input_update(SimState* sim, UIState* ui, Vector2 mouse, float dt) {
    (void)sim;
    (void)ui;
    (void)mouse;
    (void)dt;
}

void input_add_particle(SimState* sim, Vector2 pos, Vector2 vel, Color color) {
    if (sim->count >= MAX_PARTICLES) return;        // max capacity reached  

    int i = sim->count;                             // next empty slot for indexing
    sim->particles[i].pos = pos;                    // set position
    sim->particles[i].vel = vel;                    // set velocity
    sim->particles[i].color = color;                // set color
    sim->particles[i].density = 300.0f;             // same as default so that it starts as neutral state
    sim->particles[i].pressure = 0;                 // same as default so that it starts as neutral state
    sim->particles[i].force = (Vector2){0, 0};      // set force to zero
    sim->count++;                                   // count this new particle
}
