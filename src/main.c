#include "raylib.h"
#include "cascade.h"

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Cascade - SPH Fluid Simulator");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){7, 9, 26, 255});
        DrawText("hochchhe...", 600, 380, 24, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}