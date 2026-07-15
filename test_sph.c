// test_sph.c — standalone headless test for sph.c stability fixes
// Not part of the real app. Compiles separately, links only sph.o + spatial_hash.o.
//
// Build:
//   gcc -std=c99 -Wall -O2 -I./src -I/usr/local/include -c test_sph.c -o test_sph.o
//   gcc -o test_sph test_sph.o src/sph.o src/spatial_hash.o -L/usr/local/lib -lraylib -lm
// Run:
//   ./test_sph

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cascade.h"
#include "sph.h"
#include "spatial_hash.h"

#define TEST_PARTICLES 1200
#define TEST_FRAMES    300
#define TEST_DT        0.008f

int main(void) {
    srand((unsigned int)time(NULL));

    SimState sim;
    sim.count = TEST_PARTICLES;
    sim.gravity = 700.0f;
    sim.viscosity = 1.0f;
    sim.target_density = 300.0f;
    sim.particle_radius = 6.0f;
    sim.paused = false;
    sim.reset_requested = false;

    // Spawn particles in a cluster near the top of the sim area, like a "pour"
    for (int i = 0; i < sim.count; i++) {
        float rx = SIM_OFFSET_X + 200.0f + (rand() % 400);
        float ry = 20.0f + (rand() % 200);
        sim.particles[i].pos = (Vector2){ rx, ry };
        sim.particles[i].vel = (Vector2){ 0, 0 };
        sim.particles[i].force = (Vector2){ 0, 0 };
        sim.particles[i].density = 0.0f;
        sim.particles[i].pressure = 0.0f;
        sim.particles[i].color = WHITE;
    }

    static SpatialHash sh;

    printf("=== SPH stability test: %d particles, %d frames, dt=%.4f ===\n\n",
           TEST_PARTICLES, TEST_FRAMES, TEST_DT);

    int offscreen_events = 0;

    for (int frame = 1; frame <= TEST_FRAMES; frame++) {
        sh_build(&sh, sim.particles, sim.count);
        sph_compute_density(&sim, &sh);
        sph_compute_forces(&sim, &sh);
        sph_integrate(&sim, TEST_DT);

        // Track any particle that leaves the window bounds (shouldn't happen —
        // sph_integrate clamps against walls — but we check anyway as a sanity net)
        for (int i = 0; i < sim.count; i++) {
            Vector2 pos = sim.particles[i].pos;
            if (pos.x < SIM_OFFSET_X - 1 || pos.x > WINDOW_W + 1 ||
                pos.y < -1 || pos.y > WINDOW_H + 1) {
                offscreen_events++;
            }
        }

        if (frame % 30 == 0 || frame == TEST_FRAMES) {
            float density_sum = 0.0f, density_min = 1e9f, density_max = -1e9f;
            float max_speed = 0.0f;
            float y_min = 1e9f, y_max = -1e9f;

            for (int i = 0; i < sim.count; i++) {
                float d = sim.particles[i].density;
                density_sum += d;
                if (d < density_min) density_min = d;
                if (d > density_max) density_max = d;

                float vx = sim.particles[i].vel.x, vy = sim.particles[i].vel.y;
                float speed = sqrtf(vx * vx + vy * vy);
                if (speed > max_speed) max_speed = speed;

                float y = sim.particles[i].pos.y;
                if (y < y_min) y_min = y;
                if (y > y_max) y_max = y;
            }

            float density_avg = density_sum / sim.count;

            printf("frame %3d | avg density: %8.2f (target %.0f) | "
                   "density range [%.2f, %.2f] | max speed: %8.2f | "
                   "y range [%.1f, %.1f]\n",
                   frame, density_avg, sim.target_density,
                   density_min, density_max, max_speed, y_min, y_max);
        }
    }

    printf("\n=== Result ===\n");
    printf("Off-screen events over run: %d %s\n",
           offscreen_events, offscreen_events == 0 ? "(PASS)" : "(FAIL — check wall clamping)");
    printf("If avg density settled near %.0f and max speed stayed bounded (not growing\n"
           "unbounded frame to frame), stability fixes are working.\n", sim.target_density);

    return 0;
}