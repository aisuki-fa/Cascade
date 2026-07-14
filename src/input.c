#include "input.h"                                     // this file's own header (function declarations)
#include "raymath.h"                                   // Vector2Scale, Vector2Subtract
#include <stdlib.h>                                    // rand() for random particle spawn spread

// Function called once per frame: handles spawning + keyboard shortcuts
void input_update(SimState* sim, UIState* ui, ObstacleList* obs, Vector2 mouse, float dt) {
    static Vector2 last_mouse = {0};                   // persists for every frame to track mouse position

    // only spawn if mouse is in the simulation area, left click is held, and not in draw mode
    bool can_spawn = mouse.x > SIDEBAR_W && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !(ui && ui->draw_mode);
    if (can_spawn) {                                                                       // spawn legitimacy
        Color color = ui ? ui->spawn_color : (Color){135, 206, 235, 255};                  // default light blue if no UI
        Vector2 spawn_vel = Vector2Scale(Vector2Subtract(mouse, last_mouse), 3.0f);        // direction + speed * 3
        for (int i = 0; i < 4; i++) {                                                      // 4 particles per frame 
            Vector2 pos = { mouse.x + (rand() % 21 - 10), mouse.y + (rand() % 21 - 10) };  // ±10px random spread 
            input_add_particle(sim, pos, spawn_vel, color);                                // add 1 particle to simulation
        }
    }

    if (IsKeyPressed(KEY_SPACE)) sim->paused = !sim->paused;             // toggle pause (once per press)
    if (IsKeyPressed(KEY_R)) sim->reset_requested = true;                // signal main.c to clear all particles
    if (ui && IsKeyPressed(KEY_D)) ui->draw_mode = !ui->draw_mode;       // toggle wall-drawing mode

    last_mouse = mouse;                                                  // update last mouse position for next frame
}

void input_add_particle(SimState* sim, Vector2 pos, Vector2 vel, Color color) {
    if (sim->count >= MAX_PARTICLES) return;           // max capacity reached  

    int i = sim->count;                                // next empty slot for indexing
    sim->particles[i].pos = pos;                       // set position
    sim->particles[i].vel = vel;                       // set velocity
    sim->particles[i].color = color;                   // set color
    sim->particles[i].density = 300.0f;                // same as default so that it starts as neutral state
    sim->particles[i].pressure = 0;                    // same as default so that it starts as neutral state
    sim->particles[i].force = (Vector2){0, 0};         // set force to zero
    sim->count++;                                      // count this new particle
}
