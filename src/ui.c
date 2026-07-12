#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "cascade.h"
#include "ui.h"
#include "obstacles.h"

void ui_draw_sidebar(SimState* sim, UIState* ui, ObstacleList* obs) {
    DrawRectangle(0, 0, SIDEBAR_W, WINDOW_H, (Color){155, 82, 119, 255});
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, WINDOW_H, (Color){25, 40, 72, 255});
    DrawText("CASCADE", 30, 15, 32, (Color){0, 0, 0, 255});
}
