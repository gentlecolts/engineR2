#include "functions.h"
#include <algorithm>
#include <cmath>
using namespace std;

double invsqrt(double x) {
	#if 0
    double xhalf = x/2;
    long long i =*(long long*)&x;
    i = 0x5fe6eb50c7b537a9ll - (i>>1);
    x = *(double*)&i;
    x = x*(1.5 - xhalf*x*x);
	//x = x*(1.5 - xhalf*x*x);
    return x;
    #else
    return 1/sqrt(x);
    #endif
}

double sgn(double x){
	return (x>0)-(x<0);
}

/**
note that this probably does wierd stuff for large enough input or on different platforms,
but since it works well enough for the small inputs i am giving it, i will use it
*/
double pow2(long long x){
	const long long l=(0x3ff+x)<<52;
	//l<<=52;
	return *(double*)&l;
}

#if 1
void plotline(SDL_Surface* screen,int x0,int y0,int x1,int y1,uint32_t color){
	#define w (screen->w)
	#define h (screen->h)
	const int dx=x1-x0,dy=y1-y0,len=max(abs(dx),abs(dy));
	uint32_t* pixels=(uint32_t*)screen->pixels;
	if(len==0){
		if(x0>=0 && x0<w && y0>=0 && y0<h){
			pixels[x0+w*y0]=color;
		}
	}else{
		int x,y;
		for(int i=0;i<=len;i++){
			x=x0+(i*dx)/len;
			y=y0+(i*dy)/len;
			//printf("x: [%i,%i]\ty: [%i,%i]\t\tx: %i\ty: %i\n",x0,x1,y0,y1,x,y);
			if(x>=0 && x<w && y>=0 && y<h){
				pixels[x+w*y]=color;
			}
		}
	}
}

void plotlineWithDepth(SDL_Surface* target,double* depth,int x0,int y0,double d0,int x1,int y1,double d1,uint32_t color){
}
#else
void plotline(SDL_Surface *target,int x0,int y0,int x1,int y1,uint32_t color){
	uint32_t *pixels=(uint32_t*)target->pixels;

	#define w (target->w)
	#define h (target->h)

	int dx=x0-x1,dy=y0-y1,sdx=sgn(dx),sdy=sgn(dy);
	int x,y;

	if(dx==0){
		if(x0>=0 && x0<w){
			for(int n=0;n!=dy;n+=sdy){
				if(y0-n>=0 && y0-n<h){
					pixels[x0+w*(y0-n)]=color;
				}
			}
		}
	}else if(dy==0){
		if(y0>=0 && y0<h){
			for(int n=0;n!=dx;n+=sdx){
				if(x0-n>=0 && x0-n<w){
					pixels[x0-n+w*y0]=color;
				}
			}
		}
	}else{
		if(abs(dx)>=abs(dy)){
			for(int n=x0;n!=x0-dx;n-=sdx){
				y=y0-(x0-n)*dy/dx;
				if(n>=0 && n<w && y>=0 && y<h){
					pixels[n+w*y]=color;
				}
			}
		}else{
			for(int n=y0;n!=y0-dy;n-=sdy){
				x=x0-(y0-n)*dx/dy;
				if(x>=0 && x<w && n>=0 && n<h){
					pixels[x+w*n]=color;
				}
			}
		}
	}

	#undef w
	#undef h
}

void plotlineWithDepth(SDL_Surface* target,double* depth,int x0,int y0,double d0,int x1,int y1,double d1,uint32_t color){
	uint32_t *pixels=(uint32_t*)target->pixels;

	#define w (target->w)
	#define h (target->h)

	int dx=x0-x1,dy=y0-y1,sdx=sgn(dx),sdy=sgn(dy);
	int x,y;

	/*

	*/

	if(dx==0){
		if(x0>=0 && x0<w){
			for(int n=0;n!=dy;n+=sdy){
				if(y0-n>=0 && y0-n<h){
					pixels[x0+w*(y0-n)]=color;
				}
			}
		}
	}else if(dy==0){
		if(y0>=0 && y0<h){
			for(int n=0;n!=dx;n+=sdx){
				if(x0-n>=0 && x0-n<w){
					pixels[x0-n+w*y0]=color;
				}
			}
		}
	}else{
		if(abs(dx)>=abs(dy)){
			for(int n=x0;n!=x0-dx;n-=sdx){
				y=y0-(x0-n)*dy/dx;
				if(n>=0 && n<w && y>=0 && y<h){
					pixels[n+w*y]=color;
				}
			}
		}else{
			for(int n=y0;n!=y0-dy;n-=sdy){
				x=x0-(y0-n)*dx/dy;
				if(x>=0 && x<w && n>=0 && n<h){
					pixels[x+w*n]=color;
				}
			}
		}
	}

	#undef w
	#undef h
}
#endif
