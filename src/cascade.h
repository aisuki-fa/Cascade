// cascade.h — shared types for the entire Cascade project
#ifndef CASCADE_H                                      // include guard
#define CASCADE_H                                      // mark this header as included

#include "raylib.h"                                    // Vector2, Color, Font, DrawText, DrawCircle, etc.
#include <stdbool.h>                                   // bool, true, false
#include <stdlib.h>                                    // rand() for particle spawn random spread
#include <math.h>                                      // sqrt, pow, fabs

// ── Window & layout constants ─────────────────────────────────────
#define WINDOW_W        1400                           // total window width in pixels
#define WINDOW_H        800                            // total window height in pixels
#define SIDEBAR_W       220                            // left sidebar width (UI controls)
#define SIM_W           (WINDOW_W - SIDEBAR_W)         // simulation area width (1180px)
#define SIM_OFFSET_X    SIDEBAR_W                      // simulation starts at x=220

// ── Simulation limits ─────────────────────────────────────────────
#define MAX_PARTICLES   8000                           // capping particle array size
#define MAX_OBSTACLES   64                             // capping obstacle array size

// ── Core particle ─────────────────────────────────────────────────
typedef struct {
    Vector2 pos;                                       // screen-space position (x=0 left edge, y=0 top)
    Vector2 vel;                                       // velocity in pixels/sec (positive = right/down)
    Vector2 force;                                     // accumulated force vector, reset each physics step
    float   density;                                   // SPH density at this position (computed each frame)
    float   pressure;                                  // derived from density: max(0, k * (density - target))
    Color   color;                                     // user-chosen spawn color
} Particle;

// ── Simulation state (Person A owns this, others read it) ─────────
typedef struct {
    Particle particles[MAX_PARTICLES];                 // fixed array of 8000 particles
    int count;                                         // how many are alive (0 to 8000)

    // physics params (Person C sliders write to these)
    float gravity;                                     // downward force multiplier (0–1500, default 500)
    float viscosity;                                   // fluid thickness (0–5, default 0.5)
    float target_density;                              // SPH target for pressure calc (50–800, default 300)
    float particle_radius;                             // visual + collision radius in px (3–16, default 3)

    bool paused;                                       // toggling pause and resume
    bool reset_requested;                              // true = main.c calls sim_reset(), then clears
} SimState;

// ── UI state (Person C owns this, others read it) ─────────────────
typedef enum {
    RENDER_SOLID    = 0,                               // draw as spawn color (or blended with neighbors)
    RENDER_VELOCITY = 1,                               // color by speed: blue → cyan → yellow → red
    RENDER_DENSITY  = 2                                // color by density: dark → blue → white
} RenderMode;

typedef struct {
    Color       spawn_color;                           // color picker writes, input.c reads
    RenderMode  render_mode;                           // dropdown writes, renderer.c reads
    float       mouse_radius;                          // attraction/repulsion radius (later)
    bool        draw_mode;                             // true = left-click draws walls, not particles
    bool        mouse_attract;                         // right click held (later)
    bool        mouse_repel;                           // middle click held (later)
    Font        title_font;                            // custom font
} UIState;

// ── Obstacle ──────────────────────────────────────────────────────
typedef enum { OBS_RECT, OBS_CIRCLE, OBS_LINE } ObstacleType;  // shape tags

typedef struct {
    ObstacleType type;                                 // rect, circle, or line
    Vector2 p1;                                        // rect: top-left, circle: center, line: point A
    Vector2 p2;                                        // rect: width/height, circle: unused, line: point B
    float   radius;                                    // OBS_CIRCLE only
    Color   color;                                     // display color
    bool    active;                                    // false = skip collision detection
} Obstacle;

typedef struct {
    Obstacle list[MAX_OBSTACLES];                      // fixed array of 64 obstacles
    int      count;                                    // how many obstacles are active
    bool     drawing;                                  // true = user is dragging to draw a new one
    Vector2  draw_start;                               // mouse press position (start of drag)
} ObstacleList;

#endif // CASCADE_H                                    // end of include guard
