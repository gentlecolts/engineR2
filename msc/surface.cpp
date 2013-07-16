#include "surface.h"
using namespace std;

void freeSurface(surface* s){
	SDL_FreeSurface(s);
}
