#if 0
#include "vobj - old(6).cpp"
#else
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

struct nodeval{
	//double tlow=INFINITY;
	long long tlow;
	int index;
	nodeval():tlow(0x7fffffffffffffffll),index(-1){}
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

	double denom;
	//*(long long*)&denom=(0x3ffll-scale-1)<<52;
	*(long long*)&denom=(0x3ffll-scale)<<52;

	/*compute an inverse tranform for the object and apply it to the ray
	this will also change the t value, to compute the actual t,
	-find the point of intersection with the transformed coords
	-apply the object's matrix (not the inverse) to that point
	-solve for the t when the original ray is at that point (beware rounding error and inf)
	*/
	vec=v0-p;
	//*
	const double det=xvec.dot(yvec.cross(zvec)),
		vx=v.dot(yvec.cross(zvec))/det,
		vy=v.dot(xvec.cross(zvec))/det,
		vz=v.dot(xvec.cross(yvec))/det;
	double
		v0x=vec.dot(yvec.cross(zvec))/det,
		v0y=vec.dot(xvec.cross(zvec))/det,
		v0z=vec.dot(xvec.cross(yvec))/det;
	/*/
	const double det=xvec.dot(yvec.cross(zvec)),
		vx=v.x*xvec.x+v.y*yvec.x+v.z*zvec.x,
		vy=v.x*xvec.y+v.y*yvec.y+v.z*zvec.y,
		vz=v.x*xvec.z+v.y*yvec.z+v.z*zvec.z;
	double
		v0x=vec.x*xvec.x+vec.y*yvec.x+vec.z*zvec.x,
		v0y=vec.x*xvec.y+vec.y*yvec.y+vec.z*zvec.y,
		v0z=vec.x*xvec.z+vec.y*yvec.z+vec.z*zvec.z;
	//*/

	//compute intersect
	const double
		a=(1-2*(vx>=0))*denom,
		b=(1-2*(vy>=0))*denom,
		c=(1-2*(vz>=0))*denom;
	double
		x1=( a-v0x)/vx,
		x2=(-a-v0x)/vx,
		y1=( b-v0y)/vy,
		y2=(-b-v0y)/vy,
		z1=( c-v0z)/vz,
		z2=(-c-v0z)/vz;

	#define lx1 (*(long long*)&x1)
	#define lx2 (*(long long*)&x2)
	#define ly1 (*(long long*)&y1)
	#define ly2 (*(long long*)&y2)
	#define lz1 (*(long long*)&z1)
	#define lz2 (*(long long*)&z2)

	double tlow=max(max(max(lx1,ly1),lz1),0ll);

	if(*(long long*)&tlow>min(min(lx2,ly2),lz2)){
	//if(scale==0 && *(long long*)&tlow>min(min(lx2,ly2),lz2)){
		//printf("miss at scale=%i\n",scale);
		return false;
	}

	*color=0xffffff;
	return true;

	//printf("ray hits node\n");

	int
		x=(tlow*vx+v0x>0),
		y=(tlow*vy+v0y>0),
		z=(tlow*vz+v0z>0);
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
			//*closeT=*(double*)&(tlow);
			return true;
		}

		v0x=vec.dot(yvec.cross(zvec))/det;
		v0y=vec.dot(xvec.cross(zvec))/det;
		v0z=vec.dot(xvec.cross(yvec))/det;

		//t for exit planes
		x1=(a/2-v0x)/vx;
		y1=(b/2-v0y)/vy;
		z1=(c/2-v0z)/vz;

		double tlow=max(max(max(lx1,ly1),lz1),0ll);

		x+=((*(long long*)&x1)==(*(long long*)&tlow))*sgn(vx);
		y+=((*(long long*)&y1)==(*(long long*)&tlow))*sgn(vy);
		z+=((*(long long*)&z1)==(*(long long*)&tlow))*sgn(vz);
	}

	//printf("%i",scale);
	return false;
}
#endif
