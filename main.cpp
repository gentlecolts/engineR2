#include <SDL/SDL.h>
#include "engine.h"
#include <cmath>
#include <iostream>
#include <ctime>
using namespace std;

#ifdef WIN32
#include <windows.h>
void sleep(long millis){
	Sleep(millis);
}
#endif

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

	camera cam(screen,4*atan(1.0)/3,1,1,1);

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

	/*
	 shape
	bit	zyx
	0	+++
	1	++-
	2	+-+
	3	+--
	4	-++
	5	-+-
	6	--+
	7	---
	*/

	vnode *node0,*node1,*node2,*node3;
	node0=o2.head;
	node0->initChildren(0xff);

	node1=&(node0->next[0]);
	node1->initChildren(0xff);

	node2=&(node1->next[0]);
	node2->initChildren(0xfe);
	node2->next[0].color=0x00ff00;
	node2->next[1].color=0x00ff00;
	node2->next[2].color=0x00ff00;
	node2->next[3].color=0x00ff00;
	node2->next[4].color=0x00ff00;
	node2->next[5].color=0x00ff00;
	node2->next[6].color=0x00ff00;
	node2->next[7].color=0x00ff00;

	node2=&(node1->next[2]);
	node2->initChildren(0xfb);
	node2->next[0].color=0xff0000;
	node2->next[1].color=0x0000ff;
	node2->next[3].color=0xffff00;
	node2->next[4].color=0x00ff00;
	node2->next[5].color=0x00ffff;
	node2->next[6].color=0xff00ff;
	node2->next[7].color=0xffffff;

	node1=&(node0->next[1]);
	node1->initChildren(0xff);

	node2=&(node1->next[1]);
	node2->initChildren(0xfd);
	node2->next[0].color=0x00ff00;
	node2->next[1].color=0x00ff00;
	node2->next[2].color=0x00ff00;
	node2->next[3].color=0x00ff00;
	node2->next[4].color=0x00ff00;
	node2->next[5].color=0x00ff00;
	node2->next[6].color=0x00ff00;
	node2->next[7].color=0x00ff00;

	node2=&(node1->next[3]);
	node2->initChildren(0xf7);
	node2->next[0].color=0x007f00;
	node2->next[1].color=0x007f00;
	node2->next[2].color=0x007f00;
	node2->next[3].color=0x007f00;
	node2->next[4].color=0x007f00;
	node2->next[5].color=0x007f00;
	node2->next[6].color=0x007f00;
	node2->next[7].color=0x007f00;

	node1=&(node0->next[4]);
	node1->initChildren(0xff);

	node2=&(node1->next[4]);
	node2->initChildren(0xef);
	node2->next[0].color=0x00ff00;
	node2->next[1].color=0x00ff00;
	node2->next[2].color=0x00ff00;
	node2->next[3].color=0x00ff00;
	node2->next[4].color=0x00ff00;
	node2->next[5].color=0x00ff00;
	node2->next[6].color=0x00ff00;
	node2->next[7].color=0x00ff00;

	node2=&(node1->next[6]);
	node2->initChildren(0xbf);
	node2->next[0].color=0x007f00;
	node2->next[1].color=0x007f00;
	node2->next[2].color=0x007f00;
	node2->next[3].color=0x007f00;
	node2->next[4].color=0x007f00;
	node2->next[5].color=0x007f00;
	node2->next[6].color=0x007f00;
	node2->next[7].color=0x007f00;

	node1=&(node0->next[5]);
	node1->initChildren(0xff);

	node2=&(node1->next[5]);
	node2->initChildren(0xdf);
	node2->next[0].color=0x00ff00;
	node2->next[1].color=0x00ff00;
	node2->next[2].color=0x00ff00;
	node2->next[3].color=0x00ff00;
	node2->next[4].color=0x00ff00;
	node2->next[5].color=0x00ff00;
	node2->next[6].color=0x00ff00;
	node2->next[7].color=0x00ff00;

	node2=&(node1->next[7]);
	node2->initChildren(0x7f);
	node2->next[0].color=0x007f00;
	node2->next[1].color=0x007f00;
	node2->next[2].color=0x007f00;
	node2->next[3].color=0x007f00;
	node2->next[4].color=0x007f00;
	node2->next[5].color=0x007f00;
	node2->next[6].color=0x007f00;
	node2->next[7].color=0x007f00;

	node1=&(node0->next[2]);
	node1->initChildren(0xfa);

	node2=&(node1->next[1]);
	node2->initChildren(0xa0);
	node2=&(node1->next[3]);
	node2->initChildren(0xa0);
	node2=&(node1->next[4]);
	node2->initChildren(0xa0);
	node2=&(node1->next[6]);
	node2->initChildren(0xa0);

	node1=&(node0->next[3]);
	node1->initChildren(0xf5);

	node2=&(node1->next[0]);
	node2->initChildren(0x50);
	node2=&(node1->next[2]);
	node2->initChildren(0x50);
	node2=&(node1->next[5]);
	node2->initChildren(0x50);
	node2=&(node1->next[7]);
	node2->initChildren(0x50);

	node1=&(node0->next[6]);
	node1->initChildren(0xaf);

	node2=&(node1->next[0]);
	node2->initChildren(0x0a);
	node2=&(node1->next[2]);
	node2->initChildren(0x0a);
	node2=&(node1->next[5]);
	node2->initChildren(0x0a);
	node2=&(node1->next[7]);
	node2->initChildren(0x0a);

	node1=&(node0->next[7]);
	node1->initChildren(0x5f);

	node2=&(node1->next[1]);
	node2->initChildren(0x05);
	node2=&(node1->next[3]);
	node2->initChildren(0x05);
	node2=&(node1->next[4]);
	node2->initChildren(0x05);
	node2=&(node1->next[6]);
	node2->initChildren(0x05);

	o2.head->calcColors();

	printf("before write\n");
	//o2.writeToFile("data\\obj.v8l");
	printf("after write\n");
	vobj o(2,2,2,0,0,0);
	//o.readFromFile("data\\obj.v8l");
	SDL_WM_SetCaption("loading file",NULL);
	//o.readFromFile("data\\neptune_4Mtriangles_manifold\\803_neptune_4Mtriangles_manifold.off");
	//o.readFromFile("data\\WTFZOMFG\\794_lagomaggiore.off");
	//o.readFromFile("data\\Chinese_dragon\\783_Chinese_dragon.off");
	//o.readFromFile("data\\chair.off");
	o.readFromFile("data\\pleo\\pleo.off");
	//o.readFromFile("data\\pleo.v8l");
	printf("after read\n");

	SDL_WM_SetCaption("writing",NULL);

	cout<<"blah"<<endl;

	//sleep(100);
	//o.writeToFile("data\\chinese_dragon.v8l");
	//o.writeToFile("data\\pleo.v8l");

	/*
	o2.zvec.x=0.2;
	o2.yvec.z=0.2;
	o2.xvec.y=0.2;
	//*/

	SDL_Flip(screen);

	draw:
		SDL_WM_SetCaption("drawing",NULL);
		SDL_FillRect(screen,&(screen->clip_rect),0x000000);

		cam.lookAt(0,0,0);
		cam.traceScene(&o);
		cam.drawLine(0,0,0,1,0,0,0xff0000);
		cam.drawLine(0,0,0,0,1,0,0x00ff00);
		cam.drawLine(0,0,0,0,0,1,0x0000ff);
		//cam.traceScene(o[0]);
		//cam.traceScene(2,o);

		SDL_Flip(screen);

		SDL_WM_SetCaption("done",NULL);
		wait:
			chkClose();
			if(up|dn|lf|rt|fw|bw){
				#define step 0.1
				//cam.translate(step*(rt-lf),step*(up-dn),step*(fw-bw));
				cam.translateOriented(step*(rt-lf),step*(up-dn),step*(fw-bw));
				//cam.translate(step*(rt-lf),0,step*(up-dn));
				goto draw;
			}
		sleep(1);
		goto wait;
}
