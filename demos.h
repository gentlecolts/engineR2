#ifndef DEMOS_H_INCLUDED
#define DEMOS_H_INCLUDED

#include "engine.h"
#include <SDL/SDL.h>

void sleep(long millis);
void loopDrawObj(SDL_Surface* screen,vobj* o,camera& cam);
void chkClose();

void demoTesting(SDL_Surface* screen,camera& cam);
void demoMakeVobj(SDL_Surface* screen,camera& cam);
void demoVobjFromFile(SDL_Surface* screen,camera& cam);

#define bench 0

#endif // DEMOS_H_INCLUDED
