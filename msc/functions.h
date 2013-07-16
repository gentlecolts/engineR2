#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#include <SDL/SDL.h>

double invsqrt(double x);

void plotline(SDL_Surface* target,int x0,int y0,int x1,int y1,uint32_t color);
void plotlineWithDepth(SDL_Surface* target,double* depth,int x0,int y0,double d0,int x1,int y1,double d1,uint32_t color);

double sgn(double x);

double pow2(int x);

#endif // FUNCTIONS_H_INCLUDED
