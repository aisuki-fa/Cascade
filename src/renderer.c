#include "cascade.h"
#include "renderer.h"
#include<math.h>
void render_blended(SimState* sim,SpatialHash* sh );
void render_solid(SimState* sim);
void render_velocity(SimState* sim);
void render_density(SimState* sim);
void render_particles(SimState* sim, SpatialHash* sh, UIState* ui);
void render_hud(int count, int fps, bool paused);




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
    Color c;
    for(int i=0;i<sim->count;i++){
        float speed=sqrt(sim->particles[i].vel.x*sim->particles[i].vel.x+sim->particles[i].vel.y*sim->particles[i].vel.y);
        if (speed<100)
            c=lerp_color((Color){213, 196, 166}, (Color){114,43,106}, (speed)/100);
        else if(speed<=400)
            c=lerp_color((Color){114,43,106}, (Color){162,38,75}, (speed-100)/300);
        else if(speed>400){
            speed=(speed-400)/400;
            // if(speed>1)
            // speed=1;
            c=lerp_color((Color){162,38,75}, (Color){211,33,45}, speed);
    }
    DrawCircleV(sim->particles[i].pos,sim->particle_radius,c);
    }
}
void render_density(SimState* sim){
    Color c;
    for(int i=0;i<sim->count;i++){
        float t= sim->particles[i].density / sim->target_density;
        // t=fminf(fmaxf(t,0),2);
        if(t<1){
            c= lerp_color((Color){200,186,126}, (Color){78, 80, 38}, t);
        }
        else{
            c= lerp_color((Color){73, 57, 44},(Color){78, 80, 38},  1/t);
            
        }
        DrawCircleV(sim->particles[i].pos,sim->particle_radius,c);


    }

}
void render_hud(int count, int fps, bool paused) {
    (void)count;
    Color fc = fps > 50 ? GREEN : fps > 30 ? YELLOW : RED;
    DrawText(TextFormat("FPS: %d", fps), WINDOW_W - 120, 12, 30, fc);
    if (paused) {
        DrawRectangle(SIDEBAR_W, 0, SIM_W, WINDOW_H, (Color){251, 199, 191, 100});
        const char* msg = "|| PAUSED ||";
        int w = MeasureText(msg, 28);
        DrawText(msg, SIDEBAR_W + (SIM_W - w) / 2, WINDOW_H / 2 - 14, 28, (Color){255, 255, 255, 200});
    }
    else{
        DrawText(TextFormat("Ongoing"), WINDOW_W - 120, 45, 30, fc);
    }
}
void render_particles(SimState* sim, SpatialHash* sh, UIState* ui) {
    (void)sh;
        DrawLine(SIDEBAR_W, 0, SIDEBAR_W, WINDOW_H,  (Color){25,40,72,255});
    if(ui->render_mode==0)
    render_blended(sim,sh);
    else if (ui->render_mode==2)
    render_density(sim);
    else if (ui->render_mode==1)
    render_velocity(sim);

}

void render_blended(SimState* sim,SpatialHash* sh){
    for(int i=0;i<sim->count;i++){
        int out_ids[100];
        int out_count=0;
        sh_query(sh,sim->particles[i].pos.x,sim->particles[i].pos.y,out_ids,&out_count,100);
        float r=0,g=0,b=0,w=0;
        for(int j=0;j<out_count;j++){
            int id=out_ids[j];
            float distance= sqrt(pow(sim->particles[i].pos.x-sim->particles[id].pos.x,2)+pow(sim->particles[i].pos.y-sim->particles[id].pos.y,2));
            float wt= fmaxf(0,1.0f-distance/30.0f);
            r+=sim->particles[id].color.r*wt;
            g+=sim->particles[id].color.g*wt;
            b+=sim->particles[id].color.b*wt;
            w+=wt;
        }
            Color c;
            if(w>0)
            c=(Color){(int)(r/w),(int)(g/w),(int)(b/w),255};
            else c=sim->particles[i].color;
            DrawCircleV(sim->particles[i].pos,sim->particle_radius,c);
    }
}