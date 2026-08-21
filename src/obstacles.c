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

// Circle obstacle is approximated as 8 line segments forming an octagon for simplicity and performance 
void obs_add_circle(ObstacleList* obs, Vector2 c, float r) {
    if (obs->count + 8 > MAX_OBSTACLES || r < 5.0f) return;              // dragged >5px to avoid misclick, check if within MAX num of obstacles cap 
    Vector2 prev = { c.x + r, c.y };                                     // starting at 0 degrees with x-axis (right of center)
    for (int i = 1; i <= 8; i++) {                                       // octagon loop; 8 rim segments
        float a = i * 45.0f * DEG2RAD;                                   // convert degrees to radians for trig functions
        Vector2 next = { c.x + cosf(a) * r, c.y + sinf(a) * r };         // calc next rim point using polar coordinates 
        obs->list[obs->count++] = (Obstacle){OBS_LINE, prev, next, 0, GRAY, true}; // add line segment from prev to next
        prev = next;                                                     // update prev for next iteration
    }
}

// Rectangular obstacle is approximated as 4 line segments for simplicity and performance
void obs_add_rect(ObstacleList* obs, Vector2 a, Vector2 b) {
    if (obs->count + 4 > MAX_OBSTACLES || fabsf(b.x - a.x) < 5.0f || fabsf(b.y - a.y) < 5.0f) return;  // misclick and count check
    Vector2 tl = { fminf(a.x, b.x), fminf(a.y, b.y) };                                                 // top left corner (min x, min y)
    Vector2 br = { fmaxf(a.x, b.x), fmaxf(a.y, b.y) };                                                 // bottom right corner (max x, max y)
    Vector2 tr = { br.x, tl.y }, bl = { tl.x, br.y };                                                  // top right and bottom left corners
    obs->list[obs->count++] = (Obstacle){OBS_LINE, tl, tr, 0, GRAY, true};                             // top edge line drawn
    obs->list[obs->count++] = (Obstacle){OBS_LINE, tr, br, 0, GRAY, true};                             // right edge line drawn
    obs->list[obs->count++] = (Obstacle){OBS_LINE, br, bl, 0, GRAY, true};                             // bottom edge line drawn
    obs->list[obs->count++] = (Obstacle){OBS_LINE, bl, tl, 0, GRAY, true};                             // left edge line drawn
}

// Freehand wall drawing: while Draw Walls is ON, left-drag previews a cyan line and release for permanent obstacle
void obs_draw_update(ObstacleList* obs, Vector2 mouse, bool btn_down, bool btn_released, bool draw_mode) {
    int shape = obs->drop_shape;                                         // which drop tool is armed as 0=none, 1=circle, 2=rect
    if (!draw_mode && !shape) { obs->drawing = false; return; }          // no tool on cancels any half-drawn shape to prevent stale preview

    if (btn_down && !obs->drawing && mouse.x > SIDEBAR_W) {              // checks at first draw frame if mouse btn, idle state, inside sim area
        obs->drawing = true;                                             // enter drawing state
        obs->draw_start = mouse;                                         // anchor saved at press point, not drag
    }

    if (!obs->drawing) return;                                           // nothing mid-drag yet: skip preview and commit

    float dx = mouse.x - obs->draw_start.x;                              // check for rejecting accidental click-walls and
    float dy = mouse.y - obs->draw_start.y;                              // calculate drag distance in both axes
    Color preview = (Color){0,200,255,180};                              // cyan preview color shared by all three tools

    if (shape == 1) {                                                    // CIRCLE; anchor is the center, drag distance is the radius
        float r = sqrtf(dx*dx + dy*dy);                                  // distance from center to cursor 
        if (r > 5.0f) DrawCircleLines((int)obs->draw_start.x, (int)obs->draw_start.y, r, preview);  // live growing circle while dragging
        if (btn_released) obs_add_circle(obs, obs->draw_start, r);       // create octagon; helper checks obstacle cap itself

    } else if (shape == 2) {                                             // RECT: anchor and cursor are opposite corners, works in any direction
        if (fabsf(dx) > 5.0f && fabsf(dy) > 5.0f)                        // both axes must clear 5px else it's a sliver, not a box
            DrawRectangleLines((int)fminf(obs->draw_start.x, mouse.x), (int)fminf(obs->draw_start.y, mouse.y),
                               (int)fabsf(dx), (int)fabsf(dy), preview); // live outline while dragging (top-left corner + width/height)
        if (btn_released) obs_add_rect(obs, obs->draw_start, mouse);     // commit raw corners; helper normalizes and cap-checks itself

    } else {                                                            
        DrawLineEx(obs->draw_start, mouse, 3.0f, preview);               // live preview while dragging
        if (btn_released) {                                              // button released and now commit the wall
            if (obs->count < MAX_OBSTACLES && dx*dx + dy*dy > 25.0f)     // must be dragged >5px and check if it respects MAX num of obstacles cap
                obs->list[obs->count++] = (Obstacle){OBS_LINE, obs->draw_start, mouse, 0, GRAY, true}; // draw wall at first free slot, increase count
                // obs->list[obs->count++] = (Obstacle){OBS_LINE, obs->draw_start, mouse, 0, WHITE, true}; // uncomment for white instead of gray
        }
    }

    if (btn_released) obs->drawing = false;                             // back to idle state, ready for next wall or shape
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