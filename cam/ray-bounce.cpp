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
void camera::traceScene(obj* object){
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
	const double pixrad=sqrt(xvec.magSqr()/(w*w)+yvec.magSqr()/(h*h))/dir.magnitude();///what does dividing by dir's magnitude accomplish?

	#if logtime
	int rayCount=0;
	#endif

	//printf("%i\n",omp_get_dynamic());

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

	#undef w
	#undef h
	#undef expr
}
void camera::traceScene(int numobj,obj* object){
}
