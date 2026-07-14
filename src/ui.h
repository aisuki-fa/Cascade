#ifndef UI_H                                           // include guard
#define UI_H                                           // mark this header as included

#include "cascade.h"                                   // SimState, UIState types
#include "obstacles.h"                                 // ObstacleList and such

// Draws the entire left sidebar: title, controls, section backgrounds, particle counter
void ui_draw_sidebar(SimState* sim, UIState* ui, ObstacleList* obs);

#endif                                                 // end of include guard