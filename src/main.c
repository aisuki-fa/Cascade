#include<stdio.h>
#include "raylib.h"
#include "cascade.h"
#include "ui.h"
#include "renderer.h"
#include "spatial_hash.h"

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Cascade - SPH Fluid Simulator");
    SetTargetFPS(60);
    UIState ui = {0};
    ui.spawn_color = (Color){255, 255, 255, 255};
    ui.render_mode = RENDER_SOLID;
    ObstacleList obs = {0};  
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){250, 128, 114, 20});
        // TEMPORARY TEST CODE — remove after confirming render_blended works
        SimState sim;
        sim.count = 1;
        sim.particle_radius = 30; 
        SpatialHash sh;

        int dots = ((int)(GetTime() * 2) % 5); // 0 to 4, changes over time
        sim.count+=dots;
        switch(dots){
            case 4:
            sim.particles[0].pos = (Vector2){400, 450};
            sim.particles[0].color = RED;
            case 3:
            sim.particles[1].pos = (Vector2){420, 450};
            sim.particles[1].color = GREEN;
            case 2:
            sim.particles[2].pos = (Vector2){440, 450};
            sim.particles[2].color = BLUE;
            case 1:
            sim.particles[3].pos = (Vector2){460, 450};
        sim.particles[3].color = YELLOW;
             case 0:
             sim.particles[4].pos = (Vector2){480, 450};
             sim.particles[4].color = PURPLE;
             default:
              sh_build(&sh, sim.particles, sim.count);
              render_blended(&sim, &sh);
              break;
        }
        ui_draw_sidebar(&sim, &ui, &obs);
        //test code end 
        const char* suffix = dots == 0 ? "hochchhe" : dots == 1 ? "mone " : dots == 2 ? "mone toh " : dots==3? "mone toh hoy" : "mone toh hoy na";
        DrawText(TextFormat("<><><> hochchhe?%s", suffix), 300, 380, 50, WHITE);
       
        EndDrawing();
    }
    CloseWindow();
    return 0;
}