#ifndef MARCHING_SQUARES_H
#define MARCHING_SQUARES_H


#include "cascade.h"


#define MS_CELL 8
#define MS_W 175 // ceil(1180/8)
#define MS_H 100 // ceil 800/8


void ms_clear_field(float field[MS_W][MS_H]);
void ms_build_field(float field[MS_W][MS_H],SimState* sim);
void ms_draw(float field[MS_W][MS_H],Color color);
void ms_fill_blob(float field[MS_W][MS_H],Color color, float th);


#endif