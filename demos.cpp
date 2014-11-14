#include "demos.h"
using namespace std;

/*
this file includes common code to be used between the various demo programs
*/

#ifdef WIN32
#include <windows.h>
void sleep(long millis){
	Sleep(millis);
}
#endif

/*
Handle SDL's events
*/
SDL_Event e;
int key;
bool up,dn,lf,rt,fw,bw,tl,tr;
void chkClose(){
	if(SDL_PollEvent(&e)){
		switch(e.type){
		case SDL_QUIT:
			SDL_Quit();
			exit(0);
		case SDL_KEYDOWN:
			key=e.key.keysym.sym;
			if(key==SDLK_UP || key==SDLK_w){fw=true;}
			if(key==SDLK_LEFT || key==SDLK_a){lf=true;}
			if(key==SDLK_DOWN || key==SDLK_s){bw=true;}
			if(key==SDLK_RIGHT || key==SDLK_d){rt=true;}
			if(key==SDLK_i){up=true;}
			if(key==SDLK_k){dn=true;}
			if(key==SDLK_j){tl=true;}
			if(key==SDLK_l){tr=true;}
			break;
		case SDL_KEYUP:
			key=e.key.keysym.sym;
			if(key==SDLK_UP || key==SDLK_w){fw=false;}
			if(key==SDLK_LEFT || key==SDLK_a){lf=false;}
			if(key==SDLK_DOWN || key==SDLK_s){bw=false;}
			if(key==SDLK_RIGHT || key==SDLK_d){rt=false;}
			if(key==SDLK_i){up=false;}
			if(key==SDLK_k){dn=false;}
			if(key==SDLK_j){tl=false;}
			if(key==SDLK_l){tr=false;}
			break;
		}
	}
}

void loopDrawObj(SDL_Surface* screen,vobj* o,camera& cam){
	draw:
		SDL_WM_SetCaption("drawing",NULL);
		SDL_FillRect(screen,&(screen->clip_rect),0x000000);

		//cam.lookAt(0,0,0);
		//cam.pos.normalize();
		//t=clock();
		cam.traceScene(o);
		//t=clock()-t;
		//printf("time: %li\n",t);
		/*
		cam.drawLine(0,0,0,1,0,0,0xff0000);
		cam.drawLine(0,0,0,0,1,0,0x00ff00);
		cam.drawLine(0,0,0,0,0,1,0x0000ff);
		//*/
		//cam.traceScene(o[0]);
		//cam.traceScene(2,o);

		SDL_Flip(screen);

		SDL_WM_SetCaption("done",NULL);
		wait:
			chkClose();
			#if 0
			//*
			o.yvec.y=sin(clock()/1000.0);
			o.yvec.x=cos(clock()/1000.0);
			/*/
			int x,y;
			SDL_GetMouseState(&x,&y);
			o.xvec.x=2*(2*double(x)/xmax-1);
			o.yvec.y=2*(1-2*double(y)/ymax);
			//*/
			#endif
			if(up|dn|lf|rt|fw|bw|tl|tr){
				#define step 0.1
				#define dt 5
				//cam.translate(step*(rt-lf),step*(up-dn),step*(fw-bw));
				//cam.translateOriented(step*(rt-lf),step*(up-dn),step*(fw-bw));
				//cam.translate(step*(rt-lf),0,step*(up-dn));

				cam.translateOriented(0,0,step*(fw-bw));
				cam.orbitd(dt*(tr-tl),cam.getZvec().getNormalized());
				cam.orbitd(dt*(up-dn),cam.getXvec().getNormalized());
				cam.orbitd(-dt*(rt-lf),cam.getYvec().getNormalized());
				goto draw;
			}
		//sleep(1);
		#if !bench
		goto wait;
		//goto draw;
		#endif
}
