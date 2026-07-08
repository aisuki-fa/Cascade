#include "obstacles.h"

void obs_add_preset(ObstacleList* obs, PresetType type) {
    (void)obs;
    (void)type;
}

void obs_clear(ObstacleList* obs) {
    (void)obs;
}

void obs_draw_update(ObstacleList* obs, Vector2 mouse, bool btn_down, bool btn_released, bool draw_mode) {
    (void)obs;
    (void)mouse;
    (void)btn_down;
    (void)btn_released;
    (void)draw_mode;
}

void obs_render(ObstacleList* obs) {
    (void)obs;
}

void obs_resolve_collisions(ObstacleList* obs, Particle* p, int n, float radius) {
    (void)obs;
    (void)p;
    (void)n;
    (void)radius;
}
