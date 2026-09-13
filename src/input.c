#include "input.h"                                     // this file's own header (function declarations)
#include "raymath.h"                                   // Vector2Scale, Vector2Subtract
#include <stdlib.h>                                    // rand() for random particle spawn spread

// ── Spawn tuning ──────────────────────────────────────────
#define SPAWNS_PER_FRAME   4                           // particles emitted per held-frame
#define SPAWN_SPREAD       21                          // offset span; %SPREAD − SPREAD/2 → ( ±10px if spread = 21 )
#define SPAWN_VEL_SCALE    3.0f                        // mouse-move → spawn velocity multiplier
#define DEFAULT_SPAWN_COLOR ((Color){135,206,235,255}) // default light blue if no UI
#define SPAWN_DENSITY      300.0f                      // initial density default set

// ── Mouse interact tuning ────────────────────────────────
#define FORCE_RADIUS       150.0f                      // attract/repel reach (px)
#define FORCE_MIN_DIST     1.0f                        // skip particles on top of cursor
#define FORCE_SCALE        7.0f                        // force scale near cursor, 0 at radius edge

// Function called once per frame: handles spawning + keyboard shortcuts
void input_update(SimState* sim, UIState* ui, ObstacleList* obs, Vector2 mouse, float dt) {
    static Vector2 last_mouse = {0};                   // persists for every frame to track mouse position

    // only spawn if mouse is in the simulation area, left click is held, and not in draw mode, and not choosing a preset shape (circle or rect)
    bool can_spawn = mouse.x > SIDEBAR_W && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !(ui && ui->draw_mode) && !(obs && obs->drop_shape);
    if (can_spawn) {                                                                                   // spawn legitimacy
        Color color = ui ? ui->spawn_color : DEFAULT_SPAWN_COLOR;                                      // default light blue if no UI
        Vector2 spawn_vel = Vector2Scale(Vector2Subtract(mouse, last_mouse), SPAWN_VEL_SCALE);         // direction + speed * 3
        for (int i = 0; i < SPAWNS_PER_FRAME; i++) {                                                   // 4 particles per frame 
            // randomize spawn position within a square of ±10px (if SPAWN_SPREAD = 21) around the mouse cursor
            Vector2 pos = { mouse.x + (rand() % SPAWN_SPREAD - SPAWN_SPREAD/2), mouse.y + (rand() % SPAWN_SPREAD - SPAWN_SPREAD/2) }; 
            input_add_particle(sim, pos, spawn_vel, color);                                            // add 1 particle to simulation
        }
    }

    // Right click for attract and repel modes
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && mouse.x > SIDEBAR_W) {          // right click held AND cursor in sim area (not sidebar)
        bool repel = ui && ui->mouse_repel;                                      // if UI exists, check if repulsion mode is active
        for (int i = 0; i < sim->count; i++) {                                   // loop thru every particle
            Vector2 dir = repel ? Vector2Subtract(sim->particles[i].pos, mouse): // vector from mouse to particle = repel or push direction
                                  Vector2Subtract(mouse, sim->particles[i].pos); // vector from particle to mouse = attract or pull direction
            float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);                   // distance between particle and mouse 
            if (dist < FORCE_RADIUS && dist > FORCE_MIN_DIST) {                  // within FORCE_RADIUS except ones on top of cursor 
                dir.x /= dist;                                                   // normalize X to keep dir only as total vec len becomes 1 (unit vec)
                dir.y /= dist;                                                   // normalize Y to keep dir only as total vec len becomes 1 (unit vec)
                float strength = (FORCE_RADIUS - dist) * FORCE_SCALE;            // force intensity is strongest near cursor, 0 at the edge
                sim->particles[i].vel.x += dir.x * strength * dt;                // add pull to vel X (frame-rate independent)
                sim->particles[i].vel.y += dir.y * strength * dt;                // add pull to vel Y (frame-rate independent)
            }
        }
    }

    if (IsKeyPressed(KEY_SPACE)) sim->paused = !sim->paused;             // toggle pause (once per press)
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_X))              // Backspace or X = clear particles 
        sim->reset_requested = true;                                     // signal main.c to clear all particles
    if (ui) {                                                            // only check keys if ui exists (not NULL)
        if (IsKeyPressed(KEY_A)) ui->mouse_repel = false;                // Attract mode
        else if (IsKeyPressed(KEY_R)) ui->mouse_repel = true;            // Repulse mode
        if (IsKeyPressed(KEY_D)) { ui->draw_mode = !ui->draw_mode; obs->drop_shape = DROP_NONE; } // toggle wall-drawing mode (disarms any shape tool)
        if (IsKeyPressed(KEY_T)) theme_dark = !theme_dark;               // toggle dark/light theme
        if (IsKeyPressed(KEY_F)) ui->show_ms = !ui->show_ms;             // toggles fluidity overlay (marching squares) on/off
    }

    last_mouse = mouse;                                                  // update last mouse position for next frame
}

void input_add_particle(SimState* sim, Vector2 pos, Vector2 vel, Color color) {
    if (sim->count >= MAX_PARTICLES) return;           // max capacity reached  

    int i = sim->count;                                // next empty slot for indexing
    sim->particles[i].pos = pos;                       // set position
    sim->particles[i].vel = vel;                       // set velocity
    sim->particles[i].color = color;                   // set color
    sim->particles[i].density = SPAWN_DENSITY;         // set initial density
    sim->particles[i].pressure = 0;                    // same as default so that it starts as neutral state
    sim->particles[i].force = (Vector2){0, 0};         // set force to zero
    sim->particles[i].fluid_type = 0;                  // dead rn
    sim->count++;                                      // count this new particle
}
