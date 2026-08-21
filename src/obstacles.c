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

// Freehand wall drawing: while Draw Walls is ON, left-drag previews a cyan line and release for permanent obstacle
void obs_draw_update(ObstacleList* obs, Vector2 mouse, bool btn_down, bool btn_released, bool draw_mode) {
    if (!draw_mode) { obs->drawing = false; return; }                    // mode off cancels any half-drawn wall (prevents stale preview later)

    if (btn_down && !obs->drawing && mouse.x > SIDEBAR_W) {              // checks at first draw frame: mouse btn, idle state, inside sim area
        obs->drawing = true;                                             // enter drawing state
        obs->draw_start = mouse;                                         // anchor saved at press point, not drag
    }

    if (obs->drawing) {
        DrawLineEx(obs->draw_start, mouse, 3.0f, (Color){0,200,255,180});// live preview while dragging
    }

    if (btn_released && obs->drawing) {                                  // button released and now commit the wall
        float dx = mouse.x - obs->draw_start.x;                          // check for rejecting accidental click-walls and
        float dy = mouse.y - obs->draw_start.y;                          // calculate drag distance in both axes
        if (obs->count < MAX_OBSTACLES && dx*dx + dy*dy > 25.0f)         // must be dragged >5px and check if it respects MAX num of obstacles cap
            obs->list[obs->count++] = (Obstacle){OBS_LINE, obs->draw_start, mouse, 0, GRAY, true}; // draw wall at first free slot, increase count
            // obs->list[obs->count++] = (Obstacle){OBS_LINE, obs->draw_start, mouse, 0, WHITE, true}; // uncomment for white walls instead of gray
        obs->drawing = false;                                            // back to idle state, ready for next wall
    }
}

void obs_render(ObstacleList* obs) {
    for (int i = 0; i < obs->count; i++) {                                           // iterating for all the lines
        if (!obs->list[i].active) continue;                                          // skip disabled obstacles
        if (obs->list[i].type == OBS_LINE) {                                         // line segments 
            DrawLineEx(obs->list[i].p1, obs->list[i].p2, 4.0f, obs->list[i].color);  // thick 4px wall
        }
    }
}

// Particle-wall collisions: push-out along normal & bounce at 40%(changeable) strength.
void obs_resolve_collisions(ObstacleList* obs, Particle* p, int n, float radius) {
    for (int i = 0; i < obs->count; i++) {                                           // loop thru all placed walls
        if (!obs->list[i].active || obs->list[i].type != OBS_LINE) continue;         // skip inactive walls and non-line shapes
        for (int j = 0; j < n; j++) {                                                // test all particles against this wall
            Vector2 closest = closest_point_on_segment(obs->list[i].p1, obs->list[i].p2, p[j].pos); // nearest point on the wall to particle center
            Vector2 diff = { p[j].pos.x - closest.x, p[j].pos.y - closest.y };       // vector from wall surface to particle center
            float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);                   // center-to-wall distance
            if (dist < radius) {                                                     // check if particle overlaps the wall border
                if (dist < 0.001f) { diff.x = 1; diff.y = 0; dist = 1; }             // center almost on wall: +x(avoid being stuck), dist=1 avoid /0
                Vector2 normal = { diff.x / dist, diff.y / dist };                   // unit vector pointing out of the wall towards particle
                p[j].pos.x = closest.x + normal.x * (radius + 0.5f);                 // new pos at distance radius plus extra space along normal
                p[j].pos.y = closest.y + normal.y * (radius + 0.5f);
                float vDotN = p[j].vel.x * normal.x + p[j].vel.y * normal.y;         // dot product to check if approaching or not, to add bounce 
                if (vDotN < 0) {                                                     // only bounce if approaching as cos_theta is negative
                    p[j].vel.x -= 1.4f * vDotN * normal.x;                           // cancel into-wall vel (-1), add bounce (-.4), dir normal
                    p[j].vel.y -= 1.4f * vDotN * normal.y;
                }
            }
        }
    }
}