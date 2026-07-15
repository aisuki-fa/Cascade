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
// used for density calculation  h and r proportional large at r=0 density max r=h density 
// return value is a constant used for correct scaling * density scaler thats scales with distances

static inline float kernel_spiky_grad(float r, float h) {
    if (r >= h || r < 0.001f) return 0.0f;
    float h6 = h * h * h * h * h * h;
    return (-45.0f / (PI * h6)) * powf(h - r, 2) / r;
}
// repulsive force calculator divided by r 
//if particles are close r=0  high repulsive force if particles are far r=h low repulsive force



static inline float kernel_viscosity_lap(float r, float h) {
    if (r >= h) return 0.0f;
    float h5 = h * h * h * h * h;
    return (45.0f / (PI * h5)) * (h - r);
}
// used for smoothing velocity avg velocity check and splits them in the middle and returns a common velocity

// ── Density + pressure ──────────────────────────────────────────
void sph_compute_density(SimState* sim, SpatialHash* sh) {
    for (int i = 0; i < sim->count; i++) 
    // particle gulir loop
    
    {
        float density = 0.0f;
        // density of new particle so starts with 0

        int out_ids[MAX_NEIGHBORS];
        // which which particles are in the same cell as the new particle and which are in the neighboring cells
        int out_count = 0;
        // no of particles


        sh_query(sh, sim->particles[i].pos.x, sim->particles[i].pos.y,out_ids, &out_count, MAX_NEIGHBORS);

                  //"Use the (sh)to find the neighbors of the current particle (i at position x,y) and store those neighbors in out_ids."

        

        for (int j = 0; j < out_count; j++)
        // loop through all the nearby paricles of the current particle i
        {
            int n = out_ids[j];
            Vector2 diff = Vector2Subtract(sim->particles[i].pos, sim->particles[n].pos);

            // difference in coordinates of the current particle and the nearby particle

            float r_sq = Vector2DotProduct(diff, diff);
            // distance squared between the current particle and the nearby particle

            if (r_sq < SPH_H * SPH_H) {
                density += SPH_MASS * kernel_poly6(r_sq, SPH_H);
            }
            // if the distance squared is less than the smoothing radius squared, add to the density of the current particle based on the mass and the kernel function
            // otherwise density 0 aar no effect on density
            // density =m/v m is constant aar kernel poly6 calcultes volumeby distance of particles cube
        }

        sim->particles[i].density = fmaxf(density, 0.001f);
        sim->particles[i].pressure = fmaxf(0.0f, SPH_K * (density - sim->target_density));
    }
}
// guardrails density never 0 to avoid 0 division and pressure never negative to avoid attractive forces
// ── Forces: pressure + viscosity + gravity ──────────────────────
void sph_compute_forces(SimState* sim, SpatialHash* sh) {
    for (int i = 0; i < sim->count; i++) {
        sim->particles[i].force = (Vector2){ 0, 0 };

        int out_ids[MAX_NEIGHBORS];
        int out_count = 0;
        // same as prev func


        sh_query(sh, sim->particles[i].pos.x, sim->particles[i].pos.y,
                  out_ids, &out_count, MAX_NEIGHBORS);

        for (int j = 0; j < out_count; j++) {
            int n = out_ids[j];
            if (n == i) continue;
            // imp skip self force calculation

            Vector2 diff = Vector2Subtract(sim->particles[i].pos, sim->particles[n].pos); //dir + dist
            float r = Vector2Length(diff); //distance
            if (r < 0.001f) continue;

            Vector2 dir = Vector2Normalize(diff);  //direction

            float f_p = -1.0f * (sim->particles[i].pressure + sim->particles[n].pressure)
                        / (2.0f * sim->particles[n].density)
                        * kernel_spiky_grad(r, SPH_H);

                        // presure force per unit area
                        // (p[i] + p[n]) / 2  also divided by density to get force
                        // kernel calculates the force to scale with distance

            sim->particles[i].force.x += dir.x * f_p;
            sim->particles[i].force.y += dir.y * f_p;
            // scalar force theikka vector force adjusted for direction

            float f_v = sim->viscosity * SPH_MASS
                        * kernel_viscosity_lap(r, SPH_H)
                        / sim->particles[n].density;
            sim->particles[i].force.x += (sim->particles[n].vel.x - sim->particles[i].vel.x) * f_v;
            sim->particles[i].force.y += (sim->particles[n].vel.y - sim->particles[i].vel.y) * f_v;
        }
        // force=visc*mas*distancefunction/density
        // speed diff *force =force

        sim->particles[i].force.y += sim->gravity * sim->particles[i].density;
        // gravity tai shdhu y-axis e kaj korbe f=mg density used as mass replacement

        // Clamp force magnitude to prevent numerical explosion
        float mag = sqrtf(sim->particles[i].force.x * sim->particles[i].force.x
                         + sim->particles[i].force.y * sim->particles[i].force.y);
        if (mag > 5000.0f) {
            sim->particles[i].force.x = (sim->particles[i].force.x / mag) * 5000.0f;
            sim->particles[i].force.y = (sim->particles[i].force.y / mag) * 5000.0f;
        }
    }
}
// capping forces to 5k  divide by magnitude to get unit vector and multiply by 5k to get new force vector

// ── Integration: Newton's 2nd law, then move + collide ──────────
void sph_integrate(SimState* sim, float dt) {
    dt = fminf(dt, 0.008f);
    //prevtn lag
    float radius = sim->particle_radius;
    // imaginary radius jate boundary collision e particle er center theke radius distance e thake

    for (int i = 0; i < sim->count; i++) {
        Particle* pi = &sim->particles[i];

        float acc_x = pi->force.x / pi->density;
        float acc_y = pi->force.y / pi->density;
// f=ma or a=f/m density used as mass replacement
        pi->vel.x += acc_x * dt;
        pi->vel.y += acc_y * dt;
//v=v0+at
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
        // position change with velocity and time

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
// if goes out of bounds, reset position to boundary and reverse velocity with damping factor 0.4
