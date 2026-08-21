#include "input.h"                                     // this file's own header (function declarations)
#include "raymath.h"                                   // Vector2Scale, Vector2Subtract
#include <stdlib.h>                                    // rand() for random particle spawn spread

// Function called once per frame: handles spawning + keyboard shortcuts
void input_update(SimState* sim, UIState* ui, ObstacleList* obs, Vector2 mouse, float dt) {
    static Vector2 last_mouse = {0};                   // persists for every frame to track mouse position

    // only spawn if mouse is in the simulation area, left click is held, and not in draw mode, and not choosing a preset shape (circle or rect)
    bool can_spawn = mouse.x > SIDEBAR_W && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !(ui && ui->draw_mode) && !(obs && obs->drop_shape);
    if (can_spawn) {                                                                       // spawn legitimacy
        Color color = ui ? ui->spawn_color : (Color){135, 206, 235, 255};                  // default light blue if no UI
        Vector2 spawn_vel = Vector2Scale(Vector2Subtract(mouse, last_mouse), 3.0f);        // direction + speed * 3
        for (int i = 0; i < 4; i++) {                                                      // 4 particles per frame 
            Vector2 pos = { mouse.x + (rand() % 21 - 10), mouse.y + (rand() % 21 - 10) };  // ±10px random spread 
            input_add_particle(sim, pos, spawn_vel, color);                                // add 1 particle to simulation
        }
    }

    // Right click = attract
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && mouse.x > SIDEBAR_W) {              // right click held AND cursor in sim area (not sidebar)
        bool repel = ui && ui->mouse_repel;
        for (int i = 0; i < sim->count; i++) {                                       // loop thru every particle
            Vector2 dir = repel ? Vector2Subtract(sim->particles[i].pos, mouse):     // vector from particle to mouse = pull direction
                                  Vector2Subtract(mouse, sim->particles[i].pos);     // vector from mouse to particle = push direction
            float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);                       // distance between particle and mouse 
            if (dist < 150.0f && dist > 1.0f) {                                      // within 150px except ones on top of cursor 
                dir.x /= dist;                                                       // normalize X to keep dir only as total vec len becomes 1 (unit vec)
                dir.y /= dist;                                                       // normalize Y to keep dir only as total vec len becomes 1 (unit vec)
                float strength = (150.0f - dist) * 7.0f;                             // force intensity is strongest near cursor, 0 at the edge
                sim->particles[i].vel.x += dir.x * strength * dt;                    // add pull to vel X (frame-rate independent)
                sim->particles[i].vel.y += dir.y * strength * dt;                    // add pull to vel Y (frame-rate independent)
            }
        }
    }

    if (IsKeyPressed(KEY_SPACE)) sim->paused = !sim->paused;             // toggle pause (once per press)
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_X))              // Backspace or X = clear particles 
        sim->reset_requested = true;                                     // signal main.c to clear all particles
    if (ui) {                                                            // only check keys if ui exists (not NULL)
        if (IsKeyPressed(KEY_A)) ui->mouse_repel = false;                // Attract mod
        else if (IsKeyPressed(KEY_R)) ui->mouse_repel = true;            // Repulse mode
        if (IsKeyPressed(KEY_D)) { ui->draw_mode = !ui->draw_mode; obs->drop_shape = 0; } // toggle wall-drawing mode (disarms any shape tool)
    }

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
    sim->particles[i].force = (Vector2){0, 0}; 
    sim->particles[i].fluid_type = 0;                   // set force to zero
    sim->count++;                                      // count this new particle
}
