#include "obstacles.h" 
#include "raymath.h" 

// Perpendicular exit for standard approximation
// Alt to vertical drop contact point and normal bounce-off
Vector2 closest_point_on_segment(Vector2 a, Vector2 b, Vector2 p) {
    Vector2 ab = Vector2Subtract(b, a);                // ab = the surface as a position vec: go from a to b
    Vector2 ap = Vector2Subtract(p, a);                // ap = vec from end a to the particle's pos
    float t = (ab.x*ap.x + ab.y*ap.y)                  // dot product: how much of ap points along the wall
            / (ab.x*ab.x + ab.y*ab.y);                 // divide by |ab|^2 gives us fraction along wall (0 = at a, 1 = at b)
    t = fmaxf(0, fminf(1, t));                         // clamp t to [0,1] so it stays within the surface of collision
    return (Vector2){a.x + ab.x*t, a.y + ab.y*t};      //  contact point
}

void obs_add_preset(ObstacleList* obs, PresetType type) {
    int add = (type == PRESET_BOX) ? 4 : (type == PRESET_FUNNEL) ? 2 : 1;
    if (obs->count + add > MAX_OBSTACLES) return;

    if (type == PRESET_RAMP)                                                                  // ramp is a slanted surface
        obs->list[obs->count++] = (Obstacle){OBS_LINE, {320,650}, {700,450}, 0, GRAY, true};  // draw and place line obstacle in next free slot

    if (type == PRESET_BOX) {                                                                 // box; 4 lines forming a rectangle
        obs->list[obs->count++] = (Obstacle){OBS_LINE, {700,300}, {950,300}, 0, GRAY, true};  // top wall (horizontal)
        obs->list[obs->count++] = (Obstacle){OBS_LINE, {700,500}, {950,500}, 0, GRAY, true};  // bottom wall (horizontal)
        obs->list[obs->count++] = (Obstacle){OBS_LINE, {700,300}, {700,500}, 0, GRAY, true};  // left wall (vertical)
        obs->list[obs->count++] = (Obstacle){OBS_LINE, {950,300}, {950,500}, 0, GRAY, true};  // right wall (vertical)
    }

    if (type == PRESET_FUNNEL) {                                                              // funnel; 2 slanted lines sharing apex 
        obs->list[obs->count++] = (Obstacle){OBS_LINE, {550,200}, {750,520}, 0, GRAY, true};  // surface slanting down-right
        obs->list[obs->count++] = (Obstacle){OBS_LINE, {950,200}, {750,520}, 0, GRAY, true};  // surface slanting down-left
    }
}

void obs_clear(ObstacleList* obs) {
    obs->count = 0;      // clearing
}

void obs_draw_update(ObstacleList* obs, Vector2 mouse, bool btn_down, bool btn_released, bool draw_mode) {
    (void)obs;
    (void)mouse;
    (void)btn_down;
    (void)btn_released;
    (void)draw_mode;
}

void obs_render(ObstacleList* obs) {
    for (int i = 0; i < obs->count; i++) {                                           // iterating for all the lines
        if (!obs->list[i].active) continue;                                          // skip disabled obstacles
        if (obs->list[i].type == OBS_LINE) {                                         // line segments 
            DrawLineEx(obs->list[i].p1, obs->list[i].p2, 4.0f, obs->list[i].color);  // thick 4px wall
        }
    }
}

void obs_resolve_collisions(ObstacleList* obs, Particle* p, int n, float radius) {
    (void)obs;
    (void)p;
    (void)n;
    (void)radius;
}
