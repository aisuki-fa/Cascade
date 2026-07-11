#include "cascade.h"
#include "renderer.h"
#include<math.h>




Color lerp_color(Color a,Color b ,float t){
t=fminf(fmaxf(t,0),1); //clamp t to 0,1
return(Color){
    (int)(a.r +(b.r-a.r)*t),
    (int)(a.g +(b.g-a.g)*t),
    (int)(a.b +(b.b-a.b)*t),255

};}


void render_solid(SimState* sim){//renders particles into circles 
    for(int i=sim->count-1;i>=0;i--){//i for each particle 
        DrawCircleV(sim->particles[i].pos,sim->particle_radius, sim->particles[i].color);
    }
}

void render_velocity(SimState* sim){
    for(int i=0;i<sim->count;i++){
        float speed=sqrt(sim->particles[i].vel.x*sim->particles[i].vel.x+sim->particles[i].vel.y*sim->particles[i].vel.y);
        if (speed<100)
            sim->particles[i].color=lerp_color((Color){0,0,80}, (Color){135,206,235}, (speed)/100);
        else if(speed<=400)
            sim->particles[i].color=lerp_color((Color){135,206,235}, (Color){255,255,0}, (speed-100)/300);
        else if(speed>400){
            speed=(speed-400)/200;
            if(speed>1)
            speed=1;
            sim->particles[i].color=lerp_color((Color){255,255,0}, (Color){255,0,0}, speed);
    }
    DrawCircleV(sim->particles[i].pos,sim->particle_radius,sim->particles[i].color);
    }
}
void render_density(SimState* sim){
    for(int i=0;i<sim->count;i++){
        float t= sim->particles[i].density / sim->target_density;
        t=fminf(fmaxf(t,0),2);
        if(t<1){
            sim->particles[i].color= lerp_color((Color){0,0,80}, (Color){100,150,255}, t);
        }
        else{
            sim->particles[i].color= lerp_color((Color){100, 150, 255}, WHITE, t - 1);
            
        }
        DrawCircleV(sim->particles[i].pos,sim->particle_radius,sim->particles[i].color);


    }

}


void render_hud(int count, int fps, bool paused) {
    DrawText(TextFormat("Particles: %d",count), SIDEBAR_W+10,10,10,WHITE);
    Color fc= fps>50? GREEN: fps>30? YELLOW: RED;
    DrawText(TextFormat("FPS: %d",fps),SIDEBAR_W+18,30,18,fc);
    if(paused){
        DrawText("[ PAUSED ]", WINDOW_W/2-60, WINDOW_H/2, 24, PINK);
    }
}

void render_particles(SimState* sim, SpatialHash* sh, UIState* ui) {
    (void)sh;
    if(ui->render_mode==0)
    render_solid(sim);
    else if (ui->render_mode==2)
    render_density(sim);
    else if (ui->render_mode==1)
    render_velocity(sim);
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, WINDOW_H, GRAY);

}
