#ifndef SURFACE_H_INCLUDED
#define SURFACE_H_INCLUDED

#include <SDL/SDL.h>
typedef SDL_Surface surface;//temporary useage until i can write my own surface class

void freeSurface(surface* s);

#endif // SURFACE_H_INCLUDED
