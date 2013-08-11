#include <SDL/SDL.h>
#include "camera.h"
#include "../msc/functions.h"
#include <cmath>
#include <string>
#include <ctime>

#ifndef WIN64
#include <omp.h>
#endif
using namespace std;

//mtype identityarr[3][3]={{1,0,0},{0,1,0},{0,0,1}};
mtype identityarr[9]={1,0,0,0,1,0,0,0,1};
matrix I(3,3,(mtype*)identityarr);

const long double pi=4*atan(1.0l);

//this appears to be for a more mathematical xyz, where z is up
///TODO: rederive this to work with my coordinates
matrix getRotMatrix(vec3d u,double theta){
	u.normalize();
	const double c=cos(theta);
	return I*c+u.crossprodMatrix()*sin(theta)+u.tensorProd(u)*(1-c);
}

const matrix zrot90(getRotMatrix(vec3d(0,0,1),pi/2));

camera::camera(surface* target,double view_angle,double x0,double y0,double z0)://,double dirx,double diry,double dirz):
	R(getRotMatrix(vec3d(0,0,1),0)),
	scrn(target),pixels((uint32_t*)target->pixels),
	aspctRatio(double(target->h)/target->w),width(1),
	dir(vec3d(0,0,1)),
	xvec(vec3d(1,0,0)),
	yvec(vec3d(0,1,0)),
	pos(vec3d(x0,y0,z0)),
	bgCol(0x000000)
	//bgCol(0xff00ff)
{
	setViewangle(view_angle);//this sets dir

	depthbuf=new double[(scrn->w)*(scrn->h)];
}

point2d camera::getCoords(double x,double y,double z,bool* visible){
	vec3d v(x,y,z),intrsct,tmp=pos-dir;
	point2d p;
	v-=tmp;
	//v.normalize();//by doing this, t becomes the distance

	/*
	vectors: n,p,v,vp
	n - plane's normal
	v - vector in question
	p - point on plane
	v0 - origin of v

	given: p dot n = 0, p=v0+t*v
	find t

	(v0+t*v) dot n = 0
	(v0 dot n) + (t*v dot n) = 0
	t*(v dot n) = -(v0 dot n)
	t=-(v0 dot n)/(v dot n)
	*/

	const double t=
		-(dir.dot(tmp))/(dir.dot(v));
		//(dir.dot(tmp))/(dir.dot(v));

		//-(dir.dot(v))/(dir.dot(tmp));
		//(dir.dot(v))/(dir.dot(tmp));
	if(visible!=NULL){
		*visible=(t>=0);
	}
	intrsct=tmp+v*t-pos;

	p.x=intrsct.dot(xvec)*xvec.invMagSqr();
	//p.x=intrsct.dot(xvec)/(width*width/4);
	//p.x=intrsct.dot(xvec)*4/(width*width);
	p.y=intrsct.dot(yvec)*yvec.invMagSqr();
	//p.y=intrsct.dot(yvec)/(aspctRatio*aspctRatio*width*width/4);
	//p.y=intrsct.dot(yvec)*4/(aspctRatio*aspctRatio*width*width);

	return p;
}
point2d camera::getCoords(vecref v,bool* visible){
	return getCoords(v.x,v.y,v.z,visible);
}
void camera::drawPoint(double x,double y,double z,uint32_t color){//TODO: make a drawPoly funciton? it will need this
	point2d p=getCoords(x,y,z);

	if(abs(p.x)<=1 && abs(p.y)<=1){
		pixels[(int)((scrn->w)*((p.x+1)/2)+(scrn->w)*((scrn->h)*(1-(p.y+1)/2)-1))]=color;
	}
}
void camera::drawPoint(vecref p,uint32_t color){
	drawPoint(p.x,p.y,p.z,color);
}
void camera::drawLine(vecref p0,vecref p1,uint32_t color){
	drawLine(p0.x,p0.y,p0.z,p1.x,p1.y,p1.z,color);
}
void camera::drawLine(double x0,double y0,double z0,double x1,double y1,double z1,uint32_t color){
	point2d
		p0=getCoords(x0,y0,z0),
		p1=getCoords(x1,y1,z1);

	plotline(scrn,(scrn->w)*((p0.x+1)/2),(scrn->h)*(1-(p0.y+1)/2)-1,(scrn->w)*((p1.x+1)/2),(scrn->h)*(1-(p1.y+1)/2)-1,color);
}

void camera::traceScene(vobj* object){
	if(object==NULL){
		return;
	}
	#define w (scrn->w)
	#define h (scrn->h)

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

	long time=clock();

	double t;
	bool test;
	uint32_t r,g,b;

	vec3d v;
	uint32_t color;
	vec3d xv;
	double falloff;
	object->updateVals();
	vec3d tmp=pos-dir;
	const double pixrad=min(xvec.magnitude()/w,yvec.magnitude()/h);

	int rayCount=0;

	#define expr ((0.2*t+0.22)*t+1)

	//printf("%i\n",omp_get_dynamic());

	#ifndef WIN64
	#pragma omp parallel for private(t,test,r,g,b,v,color,xv,falloff) reduction( + : rayCount )
	for(int i=0;i<w*h;i++){
	#else
	for(int i=0;i<w*h;i++){
	#endif
		xv=((2.0*(i%w))/w-1)*xvec;
		xv+=dir;
		v=xv;
		v+=((2.0*(h-(i/w)-1))/h-1)*yvec;

		v.normalize();//if i never have to compare adjacent pixels, then normalizing is unnessessary, though that seems unrealistic
		test=object->intersects(v,tmp,pixrad,&color,&t);
		//test=object->intersects(dir,tmp,&color,&t);//orthagonal

		rayCount+=test;

		//printf(("v:%f %f %f\ttest:"+string((test)?"true":"false")+"\n").c_str(),v[0],v[1],v[2]);

		falloff=1.0/expr;
		r=((color>>16)&0xff)*falloff;
		g=((color>>8)&0xff)*falloff;
		b=(color&0xff)*falloff;
		//*/

		//pixels[i]=test*((0xff<<24)|(r<<16)|(g<<8)|b)+(!test)*bgCol;
		pixels[i]=test*((0xff<<24)|(r<<16)|(g<<8)|b)+(1-test)*bgCol;
	}

	t=clock()-t;

	printf("number of rays that hit: %i\ttime: %lld",rayCount,t/CLOCKS_PER_SEC);

	#undef w
	#undef h
	#undef expr
}
void camera::traceScene(int numobj,vobj object[]){
}

void camera::setViewangle(double angle){
	viewangle=angle;
	focusLen=width/(2*tan(angle/2));//width is the width of the whole camera, so the width of xvec is half
	dir.normalize();
	dir*=focusLen;
}
double camera::getViewangle(){
	return viewangle;
}

void camera::setRatio(double YoverX){
	aspctRatio=YoverX;
	yvec.normalize();
	yvec*=width/2*aspctRatio;
}
void camera::setCamWidth(double wid){
	width=wid;
	xvec.normalize();
	xvec*=wid/2;
	yvec.normalize();
	yvec*=aspctRatio*width/2;
}

void camera::lookAt(double x,double y,double z){
	dir=vec3d(x-pos.x,y-pos.y,z-pos.z).getNormalized();
	//vec3d v(x-pos.x,y-pos.y,z-pos.z);

	/*
	YxZ = X
	ZxX = Y
	XxY = Z

	{a,b,c}x{x,y,z}={b*z-c*y,c*x-a*z,a*y-b*x}

	(ZxX)xZ = X
	({x,y,z}x{a,b,c})x{x,y,z}={a,b,c}
	{y*c-z*b,z*a-x*c,x*b-y*a}x{x,y,z}={a,b,c}
	{(z*a-x*c)*z-(x*b-y*a)*y,(x*b-y*a)*x-(y*c-z*b)*z,(y*c-z*b)*y-(z*a-x*c)*x}={a,b,c}

	(z*a-x*c)*z-(x*b-y*a)*y = a
	(x*b-y*a)*x-(y*c-z*b)*z = b
	(y*c-z*b)*y-(z*a-x*c)*x = c

	z^2*a-x*c*z-x*b*y+y^2*a = a
	x^2*b-y*a*x-y*c*z+z^2*b = b
	y^2*c-z*b*y-z*a*x+x^2*c = c

	(z^2+y^2)*a-x*z*c-x*y*b = a
	(x^2+z^2)*b-y*x*a-y*z*c = b
	(y^2+x^2)*c-z*y*b-z*x*a = c

	[	?	?	x-pos.x	]
	[	?	?	y-pos.y	]
	[	?	?	z-pos.z	]
	*/

	//*
	//yvec=R*vec3d(0,1,0);
	yvec=vec3d(0,1,0);
	xvec=yvec.cross(dir);
	xvec.normalize();
	yvec=dir.cross(xvec);
	yvec.normalize();

	xvec*=width/2;
	yvec*=aspctRatio*width/2;
	dir*=focusLen;
	/*/
	///doesnt work
	double incl,incl2,azm,azm2;
	dir.getAngles(&incl,&azm);
	v.getAngles(&incl2,&azm2);
	addToAngles(incl2-incl,azm2-azm);
	xvec.normalize();
	xvec*=width/2;
	yvec.normalize();
	yvec*=aspctRatio*width/2;
	//*/
}
void camera::lookInDir(vec3d& v){
	dir=v;
	dir.normalize();

	yvec=R*vec3d(0,1,0);
	xvec=yvec.cross(dir);
	xvec.normalize();
	yvec=dir.cross(xvec);
	yvec.normalize();

	xvec*=width/2;
	yvec*=aspctRatio*width/2;
	dir*=focusLen;
}

/*
r=sqrt(x*x+y*y+z*z)
i=asin(y/r)
a=atan(z/x)

z=r*cos(i)*sin(a)
x=r*cos(i)*cos(a)
y=r*sin(i)

could this work? angles would add, but does it draw correctly?
y=r*sin(i)
z=x*tan(a) -> z=r*sin(a)
x=r*cos(a)

if they were normalized before being multiplied by r, it might:
n=r/sqrt(x^2+y^2+z^2)
=1/sqrt(sin(i)^2+sin(a)^2+cos(a)^2)
=1/sqrt(sin(i)^2+1)

x=r*cos(a)/sqrt(sin(i)^2+1)
y=r*sin(i)/sqrt(sin(i)^2+1)
z=r*sin(a)/sqrt(sin(i)^2+1)

my final results:
x=r*cos(a)/sqrt(tan(i)^2+1)=
y=r*tan(i)/sqrt(tan(i)^2+1)=
z=r*sin(a)/sqrt(tan(i)^2+1)=

y/r=tan(i)/sqrt(tan(i)^2+1)=1/sqrt(1+1/tan(i)^2)
sqrt(1+1/tan(i)^2)=r/y
1+1/tan(i)^2=(r/y)^2
tan(i)=1/sqrt((r/y)^2-1)=y/sqrt(r^2-y^2) //important note: r will be greater than or equal to y because it is the radius
i=atan(y/sqrt(r^2-y^2))=atan2(y,sqrt(r^2-y^2))

z/x=sin(a)/cos(a)=tan(a)
a=atan(z/x)=atan2(z,x)

also note: since tan^2(x)+1=sec^2(x), we arrive at
x=r*cos(a)*cos(i)
y=r*sin(i)
z=r*sin(a)*cos(i)

i=atan(y/sqrt(r^2-y^2))=asin(y/r)
*/

void camera::setIncline(double theta){
	const double a=getAzimuth();
	vec3d v(cos(a)*cos(theta),sin(theta),sin(a)*cos(theta));
	lookInDir(v);
}
void camera::setAzimuth(double theta){
	const double i=getIncline();
	vec3d v(cos(theta)*cos(i),sin(i),sin(theta)*cos(i));
	lookInDir(v);
}
void camera::setTilt(double theta){
	tilt=theta;
	R=getRotMatrix(vec3d(0,0,1),theta);

	yvec=R*vec3d(0,1,0);
	xvec=yvec.cross(dir);
	xvec.normalize();
	yvec=dir.cross(xvec);
	yvec.normalize();

	xvec*=width/2;
	yvec*=aspctRatio*width/2;
}


/*
a=atan(z/x)
i=asin(y/r)

---azimuth is increased---

x'=r*cos(a+t)*cos(i)
y'=r*sin(i)
z'=r*sin(a+t)*cos(i)

x'=r*cos(a+t)*cos(i)
x'=r*cos(atan(z/x)+t)*cos(asin(y/r))
x'=r*(cos(atan(z/x))*cos(t)-sin(atan(z/x))*sin(t))*sqrt(r*r-y*y)/r
x'=r/r*(x/sqrt(x*x+z*z)*cos(t)-z/sqrt(x*x+z*z)*sin(t))*sqrt(r*r-y*y)
note that sqrt(r*r-y*y)=sqrt(x*x+y*y+z*z-y*y)=sqrt(x*x+z*z)
x'=x*cos(t)-z*sin(t)

y'=y

z'=r*sin(a+t)*cos(i)
z'=r*sin(atan(z/x)+t)*cos(asin(y/r))
z'=r*(sin(atan(z/x))*cos(t)+cos(atan(z/x))*sin(t))*sqrt(r*r-y*y)/r
z'=r/r*(z/sqrt(x*x+z*z)*cos(t)+x/sqrt(x*x+z*z)*sin(t))*sqrt(x*x+z*z)
z'=z*cos(t)+x*sin(t)

x'=x*cos(t)-z*sin(t)
y'=y
z'=z*cos(t)+x*sin(t)


---incline is increased---
x'=r*cos(a)*cos(i+t)
y'=r*sin(i+t)
z'=r*sin(a)*cos(i+t)

x'=r*cos(atan(z/x))*cos(asin(y/r)+t)
x'=r*x/sqrt(x*x+z*z)*(cos(asin(y/r))*cos(t)-sin(asin(y/r))*sin(t))
x'=r*x/sqrt(x*x+z*z)*(sqrt(r*r-y*y)/r*cos(t)-y/r*sin(t))
x'=r*x/sqrt(x*x+z*z)*(sqrt(x*x+z*z)*cos(t)-y*sin(t))/r
x'=r/r*x*(sqrt(x*x+z*z)/sqrt(x*x+z*z)*cos(t)-y/sqrt(x*x+z*z)*sin(t))
x'=x*(cos(t)-y/sqrt(x*x+z*z)*sin(t))

y'=r*sin(asin(y/r)+t)
y'=r*(sin(asin(y/r))*cos(t)+cos(asin(y/r))*sin(t))
y'=r*(y/r*cos(t)+sqrt(r*r-y*y)/r*sin(t))
y'=y*cos(t)+sqrt(r*r-y*y)*sin(t)
y'=y*cos(t)+sqrt(x*x+z*z)*sin(t)

z'=r*sin(atan(z/x))*cos(asin(y/r)+t)
z'=r*z/sqrt(x*x+z*z)*(cos(asin(y/r))*cos(t)-sin(asin(y/r))*sin(t))
z'=r*z/sqrt(x*x+z*z)*(sqrt(x*x+z*z)*cos(t)-y*sin(t))/r
z'=r/r*z*(sqrt(x*x+z*z)/sqrt(x*x+z*z)*cos(t)-y/sqrt(x*x+z*z)*sin(t))
z'=z*(cos(t)-y/sqrt(x*x+z*z)*sin(t))

x'=x*(cos(t)-y/sqrt(x*x+z*z)*sin(t))
y'=y*cos(t)+sqrt(x*x+z*z)*sin(t)
z'=z*(cos(t)-y/sqrt(x*x+z*z)*sin(t))


---both change---
x'=r*cos(a+t)*cos(i+u)
y'=r*sin(i+u)
z'=r*sin(a+t)*cos(i+u)

x'=r*cos(atan(z/x)+t)*cos(asin(y/r)+u)
x'=r*(cos(atan(z/x))*cos(t)-sin(atan(z/x))*sin(t))*(sqrt(x*x+z*z)*cos(u)-y*sin(u))/r
x'=r/r*(x/sqrt(x*x+z*z)*cos(t)-z/sqrt(x*x+z*z)*sin(t))*(sqrt(x*x+z*z)*cos(u)-y*sin(u))
x'=(x*cos(t)-z*sin(t))/sqrt(x*x+z*z)*(sqrt(x*x+z*z)*cos(u)-y*sin(u))
x'=(x*cos(t)-z*sin(t))*(sqrt(x*x+z*z)/sqrt(x*x+z*z)*cos(u)-y/sqrt(x*x+z*z)*sin(u))
x'=(x*cos(t)-z*sin(t))*(cos(u)-y/sqrt(x*x+z*z)*sin(u))

y'=r*sin(asin(y/r)+u)
y'=r*(sin(asin(y/r))*cos(u)+cos(asin(y/r))*sin(u))
y'=r*(y/r*cos(u)+sqrt(r*r-y*y)/r*sin(u))
y'=y*cos(u)+sqrt(r*r-y*y)*sin(u)
y'=y*cos(u)+sqrt(x*x+z*z)*sin(u)

z'=r*sin(atan(z/x)+t)*cos(asin(y/r)+u)
z'=r*(sin(atan(z/x))*cos(t)+cos(atan(z/x))*sin(t))*(sqrt(x*x+z*z)*cos(u)-y*sin(u))/r
z'=r/r*(z/sqrt(x*x+z*z)*cos(t)+x/sqrt(x*x+z*z)*sin(t))*(sqrt(x*x+z*z)*cos(u)-y*sin(u))
z'=(z*cos(t)+x*sin(t))/sqrt(x*x+z*z)*(sqrt(x*x+z*z)*cos(u)-y*sin(u))
z'=(z*cos(t)+x*sin(t))*(cos(u)-y/sqrt(x*x+z*z)*sin(u))

x'=(x*cos(t)-z*sin(t))*(cos(u)-y/sqrt(x*x+z*z)*sin(u))
y'=y*cos(u)+sqrt(x*x+z*z)*sin(u)
z'=(z*cos(t)+x*sin(t))*(cos(u)-y/sqrt(x*x+z*z)*sin(u))
*/
void camera::addIncline(double theta){
	const double c=cos(theta),s=sin(theta),r=invsqrt(dir.x*dir.x+dir.z*dir.z);
	vec3d v(dir.x*(c-dir.y*r*s),dir.y*c+s/r,dir.z*(c-dir.y*r*s));
	lookInDir(v);
}
void camera::addAzimuth(double theta){
	const double c=cos(theta),s=sin(theta);
	vec3d v(dir.x*c-dir.z*s,dir.y,dir.z*c+dir.x*s);
	lookInDir(v);
}
void camera::addTilt(double theta){
	setTilt(tilt+theta);
}

void camera::setAngles(double incl,double azim){
	vec3d v(cos(azim)*cos(incl),sin(incl),sin(azim)*cos(incl));
	lookInDir(v);
}
void camera::setAngles(double incl,double azim,double tlt){
	setAngles(incl,azim);
	setTilt(tlt);
}

void camera::addToAngles(double incl,double azim){
	const double ci=cos(incl),si=sin(incl),ca=cos(azim),sa=sin(azim),r=invsqrt(dir.x*dir.x+dir.y*dir.y);
	dir=vec3d((dir.x*ca-dir.z*sa)*(ci-dir.y*r*si),dir.y*ci+si/r,(dir.z*ca+dir.x*sa)*(ci-dir.y*r*si));
	xvec=vec3d((xvec.x*ca-xvec.z*sa)*(ci-xvec.y*r*si),xvec.y*ci+si/r,(xvec.z*ca+xvec.x*sa)*(ci-xvec.y*r*si));
	yvec=vec3d((yvec.x*ca-yvec.z*sa)*(ci-yvec.y*r*si),yvec.y*ci+si/r,(yvec.z*ca+yvec.x*sa)*(ci-yvec.y*r*si));
}
void camera::addToAngles(double incl,double azim,double tlt){
	addToAngles(incl,azim);
	setTilt(tilt+tlt);
}

///I WILL NOT BE STORING INCLINE OR AZIMUTH, THESE SHOULD BE CALCULATED
double camera::getIncline(){}
double camera::getAzimuth(){}
double camera::getTilt(){return tilt;}

void camera::translate(double x,double y,double z){
	pos+=vec3d(x,y,z);
}
void camera::translate(vecref v){
	pos+=v;
}
void camera::translateOriented(double x,double y,double z){
	pos+=x*(xvec.getNormalized());
	pos+=y*(yvec.getNormalized());
	pos+=z*(dir.getNormalized());
}
void camera::translateOriented(vecref v){
	pos+=v.x*(xvec.getNormalized());
	pos+=v.y*(yvec.getNormalized());
	pos+=v.z*(dir.getNormalized());
}
void camera::moveTo(double x,double y,double z){
	pos=vec3d(x,y,z);
}
void camera::moveTo(vecref v){
	pos=v;
}

vec3d camera::getXvec(){
	return xvec;
}
vec3d camera::getYvec(){
	return yvec;
}
vec3d camera::getZvec(){
	return dir;
}
