#include <SDL/SDL.h>
//#include <SDL2/SDL.h>//TODO: actually do this at some point
#include <cmath>
#include <iostream>
#include <ctime>
#include "demos.h"
#include <cstdlib>
using namespace std;

#if bench
#define screenres 6
#else
#define screenres 2
#endif

#if screenres==0
#define xmax 320
#define ymax 240
#elif screenres==1
#define xmax 480
#define ymax 360
#elif screenres==2
#define xmax 640
#define ymax 480
#elif screenres==3
#define xmax 600
#define ymax 600
#elif screenres==4
#define xmax 700
#define ymax 700
#elif screenres==5
#define xmax 800
#define ymax 800
#elif screenres==6
#define xmax 1280
#define ymax 720
#endif

SDL_Surface* screen;
uint32_t* pixels;

int main(int argc,char** argv){
	putenv("SDL_VIDEO_WINDOW_POS=center");
	putenv("SDL_VIDEO_CENTERED=1");
	SDL_Init(SDL_INIT_EVERYTHING);

	#if 1
	screen=SDL_SetVideoMode(xmax,ymax,32,SDL_HWACCEL|SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_ASYNCBLIT);
	pixels=(uint32_t*)screen->pixels;
	#else
	SDL_Window *win=SDL_CreateWindow("Engine test",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,xmax,ymax,0);
	SDL_Renderer *rer=SDL_CreateRenderer(win,-1,0);
	#endif

	SDL_Flip(screen);
	long t;

	#if bench
	camera cam(screen,4*atan(1.0)/3,0,0,1.1);
	#else
	camera cam(screen,4*atan(1.0)/3,0,0,-2);
	//camera cam(screen,4*atan(1.0)/3,2,2,-2);
	//camera cam(screen,4*atan(1.0)/3,0,0,-100);
	#endif

	//cam.pos.normalize();
	//cam.lookAt(0,0,0);

	//cam.orbitd(180,yaxis,cam.pos);//bad, doesnt work because refrences, fix later
	//cam.rotated(180,yaxis);
	//cam.rotated(45,yaxis);


	//demoTesting(screen,cam);
	//demoMakeVobj(screen,cam);
	demoVobjFromFile(screen,cam);

	return 0;
}
