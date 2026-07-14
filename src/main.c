#include <math.h>
#include "raylib.h"
#include "raygui.h"
#include "cascade.h"
#include "spatial_hash.h"
#include "sph.h"
#include "renderer.h"
#include "input.h"
#include "ui.h"
#include "obstacles.h"

static void sim_reset(SimState* sim) {
    sim->count = 0;
}

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Cascade - SPH Fluid Simulator");
    SetTargetFPS(60);

    SimState sim = {0};
    sim.gravity = 500.0f;
    sim.viscosity = 0.5f;
    sim.target_density = 300.0f;
    sim.particle_radius = 3.0f;
    sim.paused = false;
    sim.reset_requested = false;

    UIState ui = {0};
    ui.spawn_color = (Color){135, 206, 235, 255};
    ui.render_mode = RENDER_SOLID;
    ui.draw_mode = false;

    SpatialHash sh = {0};
    ObstacleList obs = {0};

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        dt = fminf(dt, 0.02f);  // cap for lag spikes

        // Handle reset
        if (sim.reset_requested) {
            sim_reset(&sim);
            sim.reset_requested = false;
        }

        // Physics step
        if (!sim.paused) {
            sh_build(&sh, sim.particles, sim.count);
            sph_compute_density(&sim, &sh);
            sph_compute_forces(&sim, &sh);
            sph_integrate(&sim, dt);
            obs_resolve_collisions(&obs, sim.particles, sim.count, sim.particle_radius);
        }

        // Input
        Vector2 mouse = GetMousePosition();
        input_update(&sim, &ui, &obs, mouse, dt);

        // Draw
        BeginDrawing();
        ClearBackground((Color){7, 9, 26, 255});
        // ClearBackground((Color){255, 255, 255, 255});

        render_particles(&sim, &sh, &ui);
        obs_render(&obs);
        obs_draw_update(&obs, mouse,
            IsMouseButtonDown(MOUSE_BUTTON_LEFT),
            IsMouseButtonReleased(MOUSE_BUTTON_LEFT),
            ui.draw_mode);
        ui_draw_sidebar(&sim, &ui, &obs);
        render_hud(sim.count, GetFPS(), sim.paused);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}