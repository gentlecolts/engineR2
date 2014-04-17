#include <SDL/SDL.h>
#include "camera.h"
#include "../msc/functions.h"
#include <cmath>
#include <string>
#include <ctime>
#include <iostream>

#include "quat.h"

#ifndef WIN64
#include <omp.h>
#endif
using namespace std;


#define logtime 0
/**TODO: clean this up and, once using the gpu, implement photon mapping
note to self: the initial phase of path tracing should be done by lightsources
*/
void camera::traceScene(vobj* object){
	if(object==NULL){
		return;
	}
	/*
	#define w (scrn->w)
	#define h (scrn->h)
	/*/
	const int w=scrn->w,h=scrn->h;
	//*/

	/*
	Range Constant Linear Quadratic
	3250, 1.0, 0.0014, 0.000007
	600, 1.0, 0.007, 0.0002
	325, 1.0, 0.014, 0.0007
	200, 1.0, 0.022, 0.0019
	160, 1.0, 0.027, 0.0028
	100, 1.0, 0.045, 0.0075
	65, 1.0, 0.07, 0.017
	50, 1.0, 0.09, 0.032
	32, 1.0, 0.14, 0.07
	20, 1.0, 0.22, 0.20
	13, 1.0, 0.35, 0.44
	7, 1.0, 0.7, 1.8
	*/
	#define expr 1//((0.0075*t+0.045)*t+1)

	#if logtime
	long long time=clock();
	#endif

	double t;
	bool test;
	uint32_t r,g,b;

	vec3d v;
	uint32_t color;
	double falloff;
	object->updateVals();
	const vec3d tmp=pos-dir;
	//const double pixrad=max(xvec.magnitude()/w,yvec.magnitude()/h)/dir.magnitude();
	const double pixrad=sqrt(xvec.magSqr()/(w*w)+yvec.magSqr()/(h*h))/dir.magnitude();//division is needed so that

	#if logtime
	int rayCount=0;
	#endif

	//printf("%i\n",omp_get_dynamic());

	#define conetrace 0

	#if conetrace
	#define blockwidth 4

	SDL_Surface* tmpscrn=SDL_CreateRGBSurface(
		SDL_HWACCEL|SDL_HWSURFACE|SDL_HWPALETTE|SDL_ASYNCBLIT,
		blockwidth,blockwidth,32,
		0x00ff0000,
		0x0000ff00,
		0x000000ff,
		0x00000000
	);

	uint32_t* tmppix=(uint32_t*)tmpscrn->pixels;
	SDL_Rect rect;
	rect.w=blockwidth;
	rect.h=blockwidth;

	int kx,ky;

	#if 1
	/*const double invrad=invsqrt(xvec.magSqr()+yvec.magSqr());
	const double conecos=
	/*/
	//dir=1/(2tan(theta)) note that both use theta/2, and therefore the division is not necessary
	//theta=atan(1/(2dir));
	const double theta=atan(dir.invMagnitude()/2);
	const double
		conesin=sin(theta*invsqrt(w*w+h*h)/2);
		conecos=cos(theta*invsqrt(w*w+h*h)/2);
	//*/

	for(int i=0;i<w*h;i+=blockwidth){
		v=dir;
		v+=((2.0*(i%w+blockwidth/2))/w-1)*xvec;
		v+=((2.0*(h-(i/w+blockwidth/2)-1))/h-1)*yvec;



		if(true){
			for(int k=0;k<blockwidth*blockwidth;k++){
				kx=i%w+(k%blockwidth);
				ky=i/w+k/blockwidth;
				//*/
				v=dir;
				v+=((2.0*kx)/w-1)*xvec;
				v+=((2.0*(h-ky-1))/h-1)*yvec;

				v.normalize();
				test=object->intersects(v,tmp,pixrad,&color,&t);

				falloff=expr;
				r=((color>>16)&0xff)*falloff;
				g=((color>>8)&0xff)*falloff;
				b=(color&0xff)*falloff;

				//pixels[kx+w*ky]=(test*((0xff<<24)|(r<<16)|(g<<8)|b))^((!test)*bgCol);
				tmppix[k]=(test*((0xff<<24)|(r<<16)|(g<<8)|b))^((!test)*bgCol);
			}
			rect.x=i%w;
			rect.y=i/w;
			SDL_BlitSurface(tmpscrn,&tmpscrn->clip_rect,scrn,&rect);
		}
	}
	/**sphere cone intersection pseudocode:
	bool SphereIntersectsCube (Sphere S, Cone K)
	{
		U = K.vertex - (Sphere.radius/K.sin)*K.axis;
		D = S.center - U;
		if( Dot(K.axis,D) >= Length(D)*k.cos )
		{
			// center is inside K''
			D = S.center - K.vertex;
			if( -Dot(K.axis,D) >= Length(D)*K.sin )
			{
				// center is inside K'' and inside K'
				return Length(D) <= S.radius;
			}
			else
			{
				// center is inside K'' and outside K''
				return true;
			}
		}
		else
		{
			// center is outside K''
			return false;
		}
	}
	*/
	#else
	const double conerad=(pixrad*blockwidth)/2;
	double dot,rad;
	int boundx,boundy;

	//#pragma omp parallel for
	for(int i=0;i<w*h;i+=blockwidth){
		v=dir;
		v+=((2.0*(i%w+blockwidth/2))/w-1)*xvec;
		v+=((2.0*(h-(i/w+blockwidth/2)-1))/h-1)*yvec;

		dot=object->pos.dot(v);
		dot*=dot;
		dot/=v.magSqr();
		rad=
			conerad/invsqrt(object->pos.magSqr()-dot)
			+
			1/invsqrt(
				object->w*object->w
				+
				object->h*object->h
				+
				object->d*object->d);
		rad*=rad;

		if(dot<=rad){
			#if 1
			for(int k=0;k<blockwidth*blockwidth;k++){
			#else
			/*
			boundx=((w-(i%w+blockwidth)>0)*(min(blockwidth,w-(i%w+blockwidth))^blockwidth))^blockwidth;
			boundy=((h-(i/w+blockwidth)>0)*(min(blockwidth,h-(i/w+blockwidth))^blockwidth))^blockwidth;
			/*/
			boundx=min(blockwidth,w-(i%w+blockwidth));
			boundy=min(blockwidth,h-(i/w+blockwidth));
			//*/
			for(int k=0;k<boundx*boundy;k++){
			#endif
				/*
				kx=i%w+(k%boundx);
				ky=i/w+k/boundx;
				/*/
				kx=i%w+(k%blockwidth);
				ky=i/w+k/blockwidth;
				//*/
				v=dir;
				v+=((2.0*kx)/w-1)*xvec;
				v+=((2.0*(h-ky-1))/h-1)*yvec;

				v.normalize();
				test=object->intersects(v,tmp,pixrad,&color,&t);

				falloff=expr;
				r=((color>>16)&0xff)*falloff;
				g=((color>>8)&0xff)*falloff;
				b=(color&0xff)*falloff;

				//pixels[kx+w*ky]=(test*((0xff<<24)|(r<<16)|(g<<8)|b))^((!test)*bgCol);
				tmppix[k]=(test*((0xff<<24)|(r<<16)|(g<<8)|b))^((!test)*bgCol);
			}
			rect.x=i%w;
			rect.y=i/w;
			SDL_BlitSurface(tmpscrn,&tmpscrn->clip_rect,scrn,&rect);
		}
	}//}
	#endif

	SDL_FreeSurface(tmpscrn);

	#else
	#ifndef WIN64
	#if logtime
	#pragma omp parallel for private(t,test,r,g,b,v,color,falloff) reduction( + : rayCount )// schedule(dynamic,32)
	for(int i=0;i<w*h;i++){
	#else
	#pragma omp parallel for private(t,test,r,g,b,v,color,falloff)// schedule(dynamic,32)
	for(int i=0;i<w*h;i++){
	#endif
	#else
	for(int i=0;i<w*h;i++){
	#endif
		/*
		v=dir;
		v+=((2.0*(i%w))/w-1)*xvec;
		v+=((2.0*(h-(i/w)-1))/h-1)*yvec;
		/*/
		#if 0
		v=vec3d(
			dir.x+((2.0*(i%w))/w-1)*xvec.x+((2.0*(h-(i/w)-1))/h-1)*yvec.x,
			dir.y+((2.0*(i%w))/w-1)*xvec.y+((2.0*(h-(i/w)-1))/h-1)*yvec.y,
			dir.z+((2.0*(i%w))/w-1)*xvec.z+((2.0*(h-(i/w)-1))/h-1)*yvec.z
		);
		#else
		v.x=dir.x+((2.0*(i%w))/w-1)*xvec.x+((2.0*(h-(i/w)-1))/h-1)*yvec.x;
		v.y=dir.y+((2.0*(i%w))/w-1)*xvec.y+((2.0*(h-(i/w)-1))/h-1)*yvec.y;
		v.z=dir.z+((2.0*(i%w))/w-1)*xvec.z+((2.0*(h-(i/w)-1))/h-1)*yvec.z;
		#endif
		//*/

		v.normalize();//if i never have to compare adjacent pixels, then normalizing is unnessessary, though that seems unrealistic
		t=INFINITY;
		test=object->intersects(v,tmp,pixrad,&color,&t);
		//test=object->intersects(dir,tmp,&color,&t);//orthagonal

		#if logtime
		rayCount+=test;
		#endif

		//printf(("v:%f %f %f\ttest:"+string((test)?"true":"false")+"\n").c_str(),v[0],v[1],v[2]);

		falloff=expr;
		//*
		r=((color>>16)&0xff)*falloff;
		g=((color>>8)&0xff)*falloff;
		b=(color&0xff)*falloff;
		/*/
		//show the depth map instead of actual colors
		r=255/(1+t*t*0.25);
		g=r;
		b=r;
		//*/

		pixels[i]=bgCol^(test*(((0xff<<24)|(r<<16)|(g<<8)|b)^bgCol));
		//pixels[i]=test*((0xff<<24)|(r<<16)|(g<<8)|b)+(1-test)*bgCol;

		//if(i%64==0){SDL_Flip(scrn);}
	}

	#if logtime
	time=clock()-time;

	printf("number of rays that hit: %i\ttime: %lli\tseconds: %llf\n",rayCount,time,double(time)/CLOCKS_PER_SEC);
	//cout<<"number of rays that hit: "<<rayCount<<"\ttime: "<<time<<"\tseconds: "<<time/CLOCKS_PER_SEC<<endl;
	#endif
	#endif

	#undef w
	#undef h
	#undef expr
}
void camera::traceScene(int numobj,vobj object[]){
}
