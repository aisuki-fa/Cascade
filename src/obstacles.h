#ifndef OBSTACLES_H
#define OBSTACLES_H

#include "cascade.h"

typedef enum { PRESET_RAMP, PRESET_BOX, PRESET_FUNNEL } PresetType;

void obs_add_preset(ObstacleList* obs, PresetType type);
void obs_clear(ObstacleList* obs);
void obs_draw_update(ObstacleList* obs, Vector2 mouse, bool btn_down, bool btn_released, bool draw_mode);
void obs_render(ObstacleList* obs);
void obs_resolve_collisions(ObstacleList* obs, Particle* p, int n, float radius);

#endif
