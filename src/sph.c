// sph.c — SPH physics: density, forces, integration
#include "raylib.h"
#include "raymath.h"
#include "cascade.h"
#include "sph.h"
#include "spatial_hash.h"
#include <stdio.h>

#define SPH_H       30.0f   // smoothing radius (must match CELL_SIZE)
#define SPH_MASS    1200000.0f
#define SPH_K       50000000.0f  // pressure stiffness
#define MAX_NEIGHBORS 100

// ── Kernels ─────────────────────────────────────────────────────
static inline float kernel_poly6(float r_sq, float h) {
    if (r_sq >= h * h) return 0.0f;
    float h2 = h * h;
    float h9 = h2 * h2 * h2 * h2 * h;
    return (315.0f / (64.0f * PI * h9)) * powf(h2 - r_sq, 3);
}

static inline float kernel_spiky_grad(float r, float h) {
    if (r >= h || r < 0.001f) return 0.0f;
    float h6 = h * h * h * h * h * h;
    return (-45.0f / (PI * h6)) * powf(h - r, 2) / r;
}

static inline float kernel_viscosity_lap(float r, float h) {
    if (r >= h) return 0.0f;
    float h5 = h * h * h * h * h;
    return (45.0f / (PI * h5)) * (h - r);
}

// ── Density + pressure ──────────────────────────────────────────
void sph_compute_density(SimState* sim, SpatialHash* sh) {
    for (int i = 0; i < sim->count; i++) {
        float density = 0.0f;
        int out_ids[MAX_NEIGHBORS];
        int out_count = 0;

        sh_query(sh, sim->particles[i].pos.x, sim->particles[i].pos.y,
                  out_ids, &out_count, MAX_NEIGHBORS);

        

        for (int j = 0; j < out_count; j++) {
            int n = out_ids[j];
            Vector2 diff = Vector2Subtract(sim->particles[i].pos, sim->particles[n].pos);
            float r_sq = Vector2DotProduct(diff, diff);

            if (r_sq < SPH_H * SPH_H) {
                density += SPH_MASS * kernel_poly6(r_sq, SPH_H);
            }
        }

        sim->particles[i].density = fmaxf(density, 0.001f);
        sim->particles[i].pressure = fmaxf(0.0f, SPH_K * (density - sim->target_density));
    }
}

// ── Forces: pressure + viscosity + gravity ──────────────────────
void sph_compute_forces(SimState* sim, SpatialHash* sh) {
    for (int i = 0; i < sim->count; i++) {
        sim->particles[i].force = (Vector2){ 0, 0 };

        int out_ids[MAX_NEIGHBORS];
        int out_count = 0;

        sh_query(sh, sim->particles[i].pos.x, sim->particles[i].pos.y,
                  out_ids, &out_count, MAX_NEIGHBORS);

        for (int j = 0; j < out_count; j++) {
            int n = out_ids[j];
            if (n == i) continue;

            Vector2 diff = Vector2Subtract(sim->particles[i].pos, sim->particles[n].pos);
            float r = Vector2Length(diff);
            if (r < 0.001f) continue;

            Vector2 dir = Vector2Normalize(diff);

            float f_p = -1.0f * (sim->particles[i].pressure + sim->particles[n].pressure)
                        / (2.0f * sim->particles[n].density)
                        * kernel_spiky_grad(r, SPH_H);
            sim->particles[i].force.x += dir.x * f_p;
            sim->particles[i].force.y += dir.y * f_p;

            float f_v = sim->viscosity * SPH_MASS
                        * kernel_viscosity_lap(r, SPH_H)
                        / sim->particles[n].density;
            sim->particles[i].force.x += (sim->particles[n].vel.x - sim->particles[i].vel.x) * f_v;
            sim->particles[i].force.y += (sim->particles[n].vel.y - sim->particles[i].vel.y) * f_v;
        }

        sim->particles[i].force.y += sim->gravity * sim->particles[i].density;

        // Clamp force magnitude to prevent numerical explosion
        float mag = sqrtf(sim->particles[i].force.x * sim->particles[i].force.x
                         + sim->particles[i].force.y * sim->particles[i].force.y);
        if (mag > 5000.0f) {
            sim->particles[i].force.x = (sim->particles[i].force.x / mag) * 5000.0f;
            sim->particles[i].force.y = (sim->particles[i].force.y / mag) * 5000.0f;
        }
    }
}

// ── Integration: Newton's 2nd law, then move + collide ──────────
void sph_integrate(SimState* sim, float dt) {
    dt = fminf(dt, 0.008f);
    float radius = sim->particle_radius;

    for (int i = 0; i < sim->count; i++) {
        Particle* pi = &sim->particles[i];

        float acc_x = pi->force.x / pi->density;
        float acc_y = pi->force.y / pi->density;

        pi->vel.x += acc_x * dt;
        pi->vel.y += acc_y * dt;

        float speed = sqrtf(pi->vel.x * pi->vel.x + pi->vel.y * pi->vel.y);
        if (speed > 800.0f) {
            pi->vel.x = (pi->vel.x / speed) * 800.0f;
            pi->vel.y = (pi->vel.y / speed) * 800.0f;
        }

        // Global damping (simulates energy loss to heat)
        pi->vel.x *= 0.995f;
        pi->vel.y *= 0.995f;

        pi->pos.x += pi->vel.x * dt;
        pi->pos.y += pi->vel.y * dt;

        if (pi->pos.x < SIM_OFFSET_X + radius) {
            pi->pos.x = SIM_OFFSET_X + radius;
            pi->vel.x *= -0.4f;
        }
        if (pi->pos.x > WINDOW_W - radius) {
            pi->pos.x = WINDOW_W - radius;
            pi->vel.x *= -0.4f;
        }
        if (pi->pos.y < radius) {
            pi->pos.y = radius;
            pi->vel.y *= -0.4f;
        }
        if (pi->pos.y > WINDOW_H - radius) {
            pi->pos.y = WINDOW_H - radius;
            pi->vel.y *= -0.4f;
        }
    }
}