// cascade.h — shared types for the entire Cascade project
#ifndef CASCADE_H
#define CASCADE_H

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

// ── Window & layout constants ─────────────────────────────────────
#define WINDOW_W        1400
#define WINDOW_H        800
#define SIDEBAR_W       220
#define SIM_W           (WINDOW_W - SIDEBAR_W)   // simulation area width
#define SIM_OFFSET_X    SIDEBAR_W                 // simulation starts here

// ── Simulation limits ─────────────────────────────────────────────
#define MAX_PARTICLES   8000
#define MAX_OBSTACLES   64

// ── Core particle ─────────────────────────────────────────────────
typedef struct {
    Vector2 pos;        // position in screen space
    Vector2 vel;        // velocity (pixels/sec)
    Vector2 force;      // accumulated forces this frame
    float   density;    // computed by SPH
    float   pressure;   // computed by SPH
    Color   color;      // user-chosen spawn color
} Particle;

// ── Simulation state (Person A owns this, others read it) ─────────
typedef struct {
    Particle particles[MAX_PARTICLES];
    int      count;

    // physics params (Person C sliders write to these)
    float gravity;
    float viscosity;
    float target_density;
    float particle_radius;

    bool paused;
    bool reset_requested;   // Person C sets true; Person A handles & clears
} SimState;

// ── UI state (Person C owns this, others read it) ─────────────────
typedef enum {
    RENDER_SOLID    = 0,
    RENDER_VELOCITY = 1,
    RENDER_DENSITY  = 2,
} RenderMode;

typedef struct {
    Color       spawn_color;
    RenderMode  render_mode;
    float       mouse_radius;   // attraction/repulsion radius
    bool        draw_mode;      // true = user is drawing walls
    bool        mouse_attract;  // right click held
    bool        mouse_repel;    // middle click / space held
} UIState;

// ── Obstacle ──────────────────────────────────────────────────────
typedef enum { OBS_RECT, OBS_CIRCLE, OBS_LINE } ObstacleType;

typedef struct {
    ObstacleType type;
    Vector2 p1;     // top-left for rect, center for circle, point A for line
    Vector2 p2;     // width/height for rect, unused for circle, point B for line
    float   radius; // for OBS_CIRCLE only
    Color   color;
    bool    active;
} Obstacle;

typedef struct {
    Obstacle list[MAX_OBSTACLES];
    int      count;
    // for line-drawing mode
    bool     drawing;
    Vector2  draw_start;
} ObstacleList;

#endif // CASCADE_H