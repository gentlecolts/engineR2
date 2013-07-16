#include <SDL/SDL.h>
#include "engine.h"
#include <cmath>
#include <iostream>
#include <ctime>
using namespace std;

#define xmax 640
#define ymax 480

SDL_Surface* screen;
uint32_t* pixels;
SDL_Event e;

int key;
bool up,dn,lf,rt,fw,bw;
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
			break;
		case SDL_KEYUP:
			key=e.key.keysym.sym;
			if(key==SDLK_UP || key==SDLK_w){fw=false;}
			if(key==SDLK_LEFT || key==SDLK_a){lf=false;}
			if(key==SDLK_DOWN || key==SDLK_s){bw=false;}
			if(key==SDLK_RIGHT || key==SDLK_d){rt=false;}
			if(key==SDLK_i){up=false;}
			if(key==SDLK_k){dn=false;}
			break;
		}
	}
}

int main(int argc,char** argv){
	putenv("SDL_VIDEO_WINDOW_POS=center");
	putenv("SDL_VIDEO_CENTERED=1");
	SDL_Init(SDL_INIT_EVERYTHING);

	screen=SDL_SetVideoMode(xmax,ymax,32,SDL_HWACCEL|SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_ASYNCBLIT);
	pixels=(uint32_t*)screen->pixels;

	vobj o2(1,1,1,0,0,0);

	camera cam(screen,4*atan(1.0)/3,0,0,1);

	/*
	cout<<sizeof(double)<<endl;
	cout<<sizeof(long long)<<endl;
	cout<<sizeof(void*)<<endl;
	//*/
	/*
	long long l=0x3ff0000000000000l;
	double d=*(double*)&l;
	long* p=(long*)&d;
	cout<<d<<endl;
	//printf("%p %p",*p,*(p+4));
	//*/
	/*
	cout<<pow2(-13)<<endl;cout<<pow2(-12)<<endl;cout<<pow2(-11)<<endl;
	cout<<pow2(-10)<<endl;cout<<pow2(-9)<<endl;cout<<pow2(-8)<<endl;
	cout<<pow2(-7)<<endl;cout<<pow2(-6)<<endl;cout<<pow2(-5)<<endl;
	cout<<pow2(-4)<<endl;cout<<pow2(-3)<<endl;cout<<pow2(-2)<<endl;
	cout<<pow2(-1)<<endl;cout<<pow2(0)<<endl;cout<<pow2(1)<<endl;
	cout<<pow2(2)<<endl;cout<<pow2(3)<<endl;cout<<pow2(4)<<endl;
	cout<<pow2(5)<<endl;cout<<pow2(6)<<endl;cout<<pow2(7)<<endl;
	cout<<pow2(8)<<endl;cout<<pow2(9)<<endl;cout<<pow2(10)<<endl;
	cout<<pow2(11)<<endl;cout<<pow2(12)<<endl;cout<<pow2(13)<<endl;
	//*/

	/*
	double
		a=1,
		b=INFINITY,
		c=(a<b)*a+(!(a<b))*b,
		d=(a>b)*a+(!(a>b))*b;
	cout<<max(a,b)<<endl;
	cout<<min(a,b)<<endl;
	cout<<d<<endl;
	cout<<c<<endl;
	cout<<1.0/c<<endl;
	cout<<(c>1)<<endl;
	cout<<(c<1)<<endl;
	cout<<(c==1)<<endl;
	//*/

	vnode* node;
	node=o2.head;
	node->initChildren(0xfe);
	//0000 0110
	//node->initChildren(0x06);

	//*
	//node=&(node->next[1]);
	node=&(o2.head->next[1]);
	node->initChildren(0xff);

	///why does this not work?
	node==&(node->next[1]);
	node->initChildren(0xfe);

	node=&(o2.head->next[2]);
	node->initChildren(0xfe);

	///why does this not work?
	node==&(node->next[1]);
	node->initChildren(0xfe);

	/*
	o2.zvec.x=0.2;
	o2.yvec.z=0.2;
	o2.xvec.y=0.2;
	//*/

	draw:
		SDL_FillRect(screen,&(screen->clip_rect),0x000000);

		cam.lookAt(0,0,0);
		cam.traceScene(&o2);
		cam.drawLine(0,0,0,1,0,0,0xff0000);
		cam.drawLine(0,0,0,0,1,0,0x00ff00);
		cam.drawLine(0,0,0,0,0,1,0x0000ff);
		//cam.traceScene(o[0]);
		//cam.traceScene(2,o);

		SDL_Flip(screen);

		wait:
			chkClose();
			if(up|dn|lf|rt|fw|bw){
				#define step 0.2
				//cam.translate(step*(rt-lf),step*(up-dn),step*(fw-bw));
				cam.translateOriented(step*(rt-lf),step*(up-dn),step*(fw-bw));
				//cam.translate(step*(rt-lf),0,step*(up-dn));
				goto draw;
			}
			goto wait;
}
