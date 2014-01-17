#include "vobj.h"
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace std;

vobj::vobj(double wid,double hght,double dep,double x0,double y0,double z0):pos(x0,y0,z0),xvec(wid,0,0),yvec(0,hght,0),zvec(0,0,dep){
	head=new vnode();
}

//this should copy the tree of the inputted object
vobj::vobj(vobjref o){
}

vobj::~vobj(){
	delete head;
}

//this should behave like the copy constructor
vobjref vobj::operator =(vobjref o){
}

void vobj::updateVals(){
	w=(abs(xvec.x)+abs(yvec.x)+abs(zvec.x));
	h=(abs(xvec.y)+abs(yvec.y)+abs(zvec.y));
	d=(abs(xvec.z)+abs(yvec.z)+abs(zvec.z));

	rsqr=w*w+h*h+d*d;

	//#ifdef WIN64
	vec3d tmparr[6];
	tmparr[0]=xvec;
	tmparr[1]=-xvec;
	tmparr[2]=yvec;
	tmparr[3]=-yvec;
	tmparr[4]=zvec;
	tmparr[5]=-zvec;

	for(int i=0;i<8;i++){
		varr[i]=tmparr[i&0x1];
		varr[i]+=tmparr[2+((i>>1)&0x1)];
		varr[i]+=tmparr[4+((i>>2)&0x1)];
	}
	/*
	#else
	varr={xvec,-xvec,yvec,-yvec,zvec,-zvec};
	#endif
	*/
}

bool vobj::intersects(vecref vec,vecref origin,const double pixrad,uint32_t* color,double* closeT) const{
	vec3d v0=origin;//copy the vector so that the original can be modified if this was called in parallel
	return chkIntersect(head,pos,vec,v0,pixrad,color,0,closeT);
}
bool vobj::intersects(vecref vec,double x0,double y0,double z0,const double pixrad,uint32_t* color,double* closeT) const{
	vec3d v0(x0,y0,z0);
	return chkIntersect(head,pos,vec,v0,pixrad,color,0,closeT);
}

#define infguard0 1
#define infguard1 1
#define infguard2 0
#define infguard3 0
#define infguard4 0
/**
43210
works:
00011
00111
01011
01111
10011
10111
11011
11111
*/

struct nodeval{
	//double tlow=INFINITY;
	long long tlow=0x7fffffffffffffffll;
	int index=-1;
	nodeval(){}
	nodeval(double t,int i):tlow(t),index(i){}

	inline bool operator <(const nodeval& n) const{return *(long long*)&(tlow) < *(long long*)&(n.tlow);}
	inline bool operator <=(const nodeval& n) const{return *(long long*)&(tlow) <= *(long long*)&(n.tlow);}
	inline bool operator >(const nodeval& n) const{return *(long long*)&(tlow) > *(long long*)&(n.tlow);}
	inline bool operator >=(const nodeval& n) const{return *(long long*)&(tlow) >= *(long long*)&(n.tlow);}
	inline bool operator ==(const nodeval& n) const{return *(long long*)&(tlow) == *(long long*)&(n.tlow);}
};

///TODO: try to make this w/o recursion
bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	///TODO: storing normals might be best
	#define norm_light 0

	//if(!(node->nodes) | (node->shape==0x00)){// || pixrad*tlow*pixrad*tlow>rsqr*denom*denom){
	if(!(node->nodes) | !(node->shape)){// || pixrad*tlow*pixrad*tlow>rsqr*denom*denom){
		#if norm_light==0
		*color=node->color;
		// *color=node->color;
		#else

		const double
		//closeT is set as the tlow of this node
			x=abs((*closeT)*v.x+v0.x-p.x),
			y=abs((*closeT)*v.y+v0.y-p.y),
			z=abs((*closeT)*v.z+v0.z-p.z);
		//float dot=abs(v.xyz[(y>x && y>z)+2*(z>x && z>y)]);//*(pixrad*tlow*pixrad*tlow<=rsqr*denom*denom);
		float dot=abs(v.xyz[(*(long long*)&y>*(long long*)&x && *(long long*)&y>*(long long*)&z)+2*(*(long long*)&z>*(long long*)&x && *(long long*)&z>*(long long*)&y)]);//*(pixrad*tlow*pixrad*tlow<=rsqr*denom*denom);
		//float dot=(abs(v.x)+abs(v.y)+abs(v.z))/3;

		//dot*=(dot>0);
		dot=(dot+1)/2;
		//dot=1-(dot-1)*(dot-1);
		const uint32_t
			r=(((node->color)>>16)&0xff)*dot,
			g=(((node->color)>>8)&0xff)*dot,
			b=((node->color)&0xff)*dot;

		*color=(r<<16)|(g<<8)|b;
		#endif

		//*closeT = tlow;

		//printf("success!/n");
		return true;
	}

	vec3d vec;

	//long long tlow,thigh;
	//long long tlow;
	double
		x1,x2,
		y1,y2,
		z1,z2;

	double denom;
	//*(long long*)&denom=(0x3ffll-scale-1)<<52;
	*(long long*)&denom=(0x3ffll-scale)<<52;

	//nodeval valarr[4];
	int numnodes=0;

	bool tmpbool;

	/*
	nx*(x-x0)+ny*(y-y0)+nz*(z-z0)=0
	nx*(dx*t+vx-x0)+ny*(dy*t+vy-y0)+nz*(dz*t+vz-z0)=0
	nx*dx*t+nx*vx-nx*x0+ny*dy*t+ny*vy-ny*y0+nz*dz*t+nz*vz-nz*z0=0
	(nx*dx+ny*dy+nz*dz)*t+nx*vx-nx*x0+ny*vy-ny*y0+nz*vz-nz*z0=0
	t=((nx*x0+ny*y0+nz*z0)-(nx*vx+ny*vy+nz*vz))/(nx*dx+ny*dy+nz*dz)
	t=(nx*(x0-vx)+ny*(y0-vy)+nz*(z0-vz))/(nx*dx+ny*dy+nz*dz)
	*/

	double
		tyz=xvec.dot((p-denom*sgn(xvec.dot(v))*xvec)-v0)/xvec.dot(v),
		txz=yvec.dot((p-denom*sgn(yvec.dot(v))*yvec)-v0)/yvec.dot(v),
		txy=zvec.dot((p-denom*sgn(zvec.dot(v))*zvec)-v0)/zvec.dot(v);
	double tlow=*(double*)&min(min(*(long long*)&tyz,*(long long*)&txz),*(long long*)&txy);

	tyz=xvec.dot((p+denom*sgn(xvec.dot(v))*xvec)-v0)/xvec.dot(v);
	txz=yvec.dot((p+denom*sgn(yvec.dot(v))*yvec)-v0)/yvec.dot(v);
	txy=zvec.dot((p+denom*sgn(zvec.dot(v))*zvec)-v0)/zvec.dot(v);

	if(scale==0 && *(long long*)&tlow>max(max(*(long long*)&tyz,*(long long*)&txz),*(long long*)&txy)){
		//printf("miss at scale=%i\n",scale);
		return false;
	}

	if(scale==1){
		*color=0xffffff;
		return true;
	}

	//printf("ray hits node\n");

	int
		x=(xvec.dot(tlow*v+v0-p)>0),
		y=(yvec.dot(tlow*v+v0-p)>0),
		z=(zvec.dot(tlow*v+v0-p)>0);
	int index;

	++scale;
	denom/=2;

	while( (x==(x&1)) & (y==(y&1)) & (z==(z&1))){
		index=(z<<2)|(y<<1)|x;

		vec.x=varr[index].x*denom+p.x;
		vec.y=varr[index].y*denom+p.y;
		vec.z=varr[index].z*denom+p.z;

		if(((node->shape>>index)&0x1) && chkIntersect(
				&(node->nodes->next[index]),
				vec,
				v,v0,pixrad,color,scale,(double*)&tlow
			)
		){
			//*closeT=*(double*)&(valarr[i].tlow);
			return true;
		}

		//t for exit planes
		tyz=xvec.dot((vec-denom*sgn(xvec.dot(v))*xvec)-v0)/xvec.dot(v);
		txz=yvec.dot((vec-denom*sgn(yvec.dot(v))*yvec)-v0)/yvec.dot(v);
		txy=zvec.dot((vec-denom*sgn(zvec.dot(v))*zvec)-v0)/zvec.dot(v);

		tlow=*(double*)&min(min(*(long long*)&tyz,*(long long*)&txz),*(long long*)&txy);

		x+=((*(long long*)&tyz)==(*(long long*)&tlow))*sgn(xvec.dot(v));
		y+=((*(long long*)&txz)==(*(long long*)&tlow))*sgn(yvec.dot(v));
		z+=((*(long long*)&txy)==(*(long long*)&tlow))*sgn(zvec.dot(v));
	}

	//printf("%i",scale);
	return false;
}
