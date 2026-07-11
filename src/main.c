#include "raylib.h"
#include "cascade.h"
#include "input.h"

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Cascade - SPH Fluid Simulator");
    SetTargetFPS(60);

    SimState sim = {0};
    sim.count = 0;
    sim.paused = false;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        input_update(&sim, NULL, NULL, mouse, GetFrameTime());

        BeginDrawing();
        ClearBackground((Color){7, 9, 26, 255});
        DrawText(TextFormat("Count: %d", sim.count), 10, 10, 20, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}