#include "cascade.h"
#include "marching_squares.h"
#include <math.h>


void ms_clear_field(float field[MS_W][MS_H]);
void ms_build_field(float field[MS_W][MS_H],SimState* sim);
void ms_draw(float field[MS_W][MS_H],Color color);
void ms_fill_blob(float field[MS_W][MS_H],Color color, float th);





void ms_fill_blob(float field[MS_W][MS_H],Color color,float th) {
    for(int x=0;x<MS_W-1;x++) {
        for(int y=0;y<MS_H-1;y++){
            float f0 = field[x][y],
             f1 = field[x + 1][y],
            f2 = field[x][y + 1],
             f3 = field[x + 1][y + 1];
            if(f0>th && f1> th && f2> th && f3> th) {
                float wx = x * MS_CELL + SIDEBAR_W,
                wy = y * MS_CELL;
                DrawCircle(wx,wy,3.5f,color);
            }
        }
    }
}

void ms_clear_field(float field[MS_W][MS_H]){

        for(int x=0;x<MS_W;x++){

            for(int y=0;y<MS_H;y++){
                field[x][y]=0.0f;
            }
        }
    }



void ms_build_field(float field[MS_W][MS_H],SimState* sim){

    for(int i=0;i<sim->count;i++){
        int gx=(int) (sim->particles[i].pos.x-SIDEBAR_W)/MS_CELL;
        int gy= (int)(sim->particles[i].pos.y)/MS_CELL;
        for (int dx=-6;dx<=6;dx++)
        for (int dy=-6;dy<=6;dy++){
            int nx=gx+dx,ny=gy+dy;
            if(nx<0||nx>=MS_W||ny<0||ny>=MS_H)
            continue;
        float wx=(nx*MS_CELL + SIDEBAR_W), wy=(ny*MS_CELL);
        float dist_sq=(sim->particles[i].pos.x-wx)*(sim->particles[i].pos.x-wx) + (sim->particles[i].pos.y-wy)*(sim->particles[i].pos.y-wy);
        field[nx][ny]+=expf(-dist_sq/450);

        }
    }
}

void ms_draw(float field[MS_W][MS_H], Color color){
    float th=0.6f;
    for(int iy=0;iy<MS_H-1;iy++)
    for(int ix=0;ix<MS_W-1;ix++){
        float f0=field[ix][iy],
        f1=field[ix+1][iy],
        f2=field[ix][iy+1],
        f3=field[ix+1][iy+1];

        int code=(f3>th?8:0) | (f2>th?4:0) | (f1>th?2:0) | (f0>th?1:0);
        if (!code || code==15)
        continue;
     float tBot=(fabsf(f1-f0)<1e-6f)?0.5f:(th-f0)/(f1-f0),
             tTop=(fabsf(f3- f2)<1e-6f)?0.5f:(th-f2)/(f3-f2),
             tLeft =(fabsf(f2- f0)<1e-6f)?0.5f:(th-f0)/(f2- f0),
             tRight=(fabsf(f3-f1)<1e-6f)?0.5f:(th-f1)/(f3-f1);

             Vector2 bottom={(ix+tBot)*MS_CELL + SIDEBAR_W,iy*MS_CELL},
             top={(ix+tTop)*MS_CELL + SIDEBAR_W,(iy+1)*MS_CELL},
             left={ix*MS_CELL + SIDEBAR_W,(iy+tLeft)*MS_CELL},
             right={(ix+1)*MS_CELL + SIDEBAR_W,(iy+tRight)*MS_CELL};
             switch(code){
             case 1:  DrawLineEx(left, bottom, 2.0f, color); break;
                case 2:  DrawLineEx(bottom, right, 2.0f, color); break;
                case 3:  DrawLineEx(left, right, 2.0f, color); break;
                case 4:  DrawLineEx(top, left, 2.0f, color); break;
                case 5:  DrawLineEx(top, bottom, 2.0f, color); break;
                case 6:  DrawLineEx(left, top, 2.0f, color);
                         DrawLineEx(bottom, right, 2.0f, color); break;
                case 7:  DrawLineEx(top, right, 2.0f, color); break;
                case 8:  DrawLineEx(right, top, 2.0f, color); break;
                case 9:  DrawLineEx(left, bottom, 2.0f, color);
                         DrawLineEx(top, right, 2.0f, color); break;
                case 10: DrawLineEx(top, bottom, 2.0f, color); break;
                case 11: DrawLineEx(top, left, 2.0f, color); break;
                case 12: DrawLineEx(left, right, 2.0f, color); break;
                case 13: DrawLineEx(bottom, right, 2.0f, color); break;
                case 14: DrawLineEx(left, bottom, 2.0f, color); break;
                default: break;
            }           
    }

    
}
     


