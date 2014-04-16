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

union lldouble{
	double d;
	long long l;
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
		///TODO: fix this stupid shit
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

		return true;
	}

	vec3d vec;

	//long long tlow,thigh;
	long long tlow;
	lldouble
		x1,x2,
		y1,y2,
		z1,z2;
	#define uselongxor 0

	/*
	double denom;
	*(long long*)&denom=(0x3ffll-scale-1)<<52;
	/*/
	const lldouble denom={.l=(0x3ffll-scale-1)<<52};
	//*/

	nodeval valarr[8];
	int numnodes=0;

	bool tmpbool;

	const double
		a=(1-2*(v.x>=0))*w*denom.d,
		b=(1-2*(v.y>=0))*h*denom.d,
		c=(1-2*(v.z>=0))*d*denom.d,
		invdx=1/v.x,
		invdy=1/v.y,
		invdz=1/v.z;
	#if 0
	const double
		x1val=(a-(v0.x-p.x))*invdx,
		x2val=(-a-(v0.x-p.x))*invdx,
		y1val=(b-(v0.y-p.y))*invdy,
		y2val=(-b-(v0.y-p.y))*invdy,
		z1val=(c-(v0.z-p.z))*invdz,
		z2val=(-c-(v0.z-p.z))*invdz,
		denomx=denom*invdx,
		denomy=denom*invdy,
		denomz=denom*invdz;
	#else
	#define x1val a-(v0.x-p.x)
	#define x2val -a-(v0.x-p.x)
	#define y1val b-(v0.y-p.y)
	#define y2val -b-(v0.y-p.y)
	#define z1val c-(v0.z-p.z)
	#define z2val -c-(v0.z-p.z)
	#endif
	for(int i=0;i<8;i++){
	#if 1
		x1.d=(x1val+varr[i].x*denom.d)*invdx;
		x2.d=(x2val+varr[i].x*denom.d)*invdx;
		y1.d=(y1val+varr[i].y*denom.d)*invdy;
		y2.d=(y2val+varr[i].y*denom.d)*invdy;
		z1.d=(z1val+varr[i].z*denom.d)*invdz;
		z2.d=(z2val+varr[i].z*denom.d)*invdz;
	#else
		x1=x1val+varr[i].x*denomx;
		x2=x2val+varr[i].x*denomx;
		y1=y1val+varr[i].y*denomy;
		y2=y2val+varr[i].y*denomy;
		z1=z1val+varr[i].z*denomz;
		z2=z2val+varr[i].z*denomz;
	#endif
		/*
		#define lx1 (*(long long*)&x1)
		#define lx2 (*(long long*)&x2)
		#define ly1 (*(long long*)&y1)
		#define ly2 (*(long long*)&y2)
		#define lz1 (*(long long*)&z1)
		#define lz2 (*(long long*)&z2)
		/*/
		#define lx1 (x1.l)
		#define lx2 (x2.l)
		#define ly1 (y1.l)
		#define ly2 (y2.l)
		#define lz1 (z1.l)
		#define lz2 (z2.l)
		//*/

		#if uselongxor
		tlow=(((lx1>=ly1) & (lx1>=lz1))*lx1)^(((ly1>lx1) & (ly1>=lz1))*ly1)^(((lz1>lx1) & (lz1>ly1))*lz1);

		tlow=max(tlow,0ll);
		#else
		tlow=max(max(max(lx1,ly1),lz1),0ll);
		/*
		thigh=min(min(lx2,ly2),lz2);
		#endif

		tmpbool=( ((node->shape>>i)&0x1) & (tlow<=thigh) );
		/*/
		#endif
		//tmpbool=( ((node->shape>>i)&0x1) & (tlow<=min(min(lx2,ly2),lz2)) );
		tmpbool=( ((node->shape>>i)&0x1) & (tlow<=lx2) & (tlow<=ly2) & (tlow<=lz2) );
		//*/
		valarr[i].tlow^=tmpbool*(tlow^valarr[i].tlow);
		valarr[i].index^=tmpbool*(i^valarr[i].index);
		numnodes+=tmpbool;
	}

	/*
	max(a,b,c)<=min(d,e,f)
	0<=min(d,e,f)-max(a,b,c)
	0<=min(d-max(a,b,c),e-max(a,b,c),f-max(a,b,c))
	0<=min(d+min(-a,-b,-c),e+min(-a,-b,-c),f+min(-a,-b,-c))
	0<=min(min(d-a,d-b,d-c),min(e-a,e-b,e-c),min(f-a,f-b,f-c))
	0<=min(d-a,d-b,d-c,e-a,e-b,e-c,f-a,f-b,f-c)

	a=(w+x)/dx
	b=(h+y)/dy
	c=(d+z)/dz
	d=(-w+x)/dx
	e=(-h+y)/dy
	f=(-d+z)/dz

	0<=min(d-a,d-b,d-c,e-a,e-b,e-c,f-a,f-b,f-c)
	0<=min((-w+x)/dx-(w+x)/dx,(-w+x)/dx-(h+y)/dy,(-w+x)/dx-(d+z)/dz,(-h+y)/dy-(w+x)/dx,(-h+y)/dy-(h+y)/dy,(-h+y)/dy-(d+z)/dz,(-d+z)/dz-(w+x)/dx,(-d+z)/dz-(h+y)/dy,(-d+z)/dz-(d+z)/dz)
	0<=min((-w+x-(w+x))/dx,((-w+x)*dy-(h+y)*dx)/(dx*dy),((-w+x)*dz-(d+z)*dx)/(dx*dz),((-h+y)*dx-(w+x)*dy)/(dy*dx),(-h+y-(h+y))/dy,((-h+y)*dz-(d+z)*dy)/(dy*dz),((-d+z)*dx-(w+x)*dz)/(dz*dx),((-d+z)*dy-(h+y)*dz)/(dz*dy),(-d+z-(d+z))/dz)
	0<=min(
			-2*w/dx,
			-2*h/dy,
			-2*d/dz,
			((-w+x)*dy-(h+y)*dx)/(dx*dy),
			((-h+y)*dx-(w+x)*dy)/(dx*dy),
			((-w+x)*dz-(d+z)*dx)/(dx*dz),
			((-d+z)*dx-(w+x)*dz)/(dx*dz),
			((-h+y)*dz-(d+z)*dy)/(dy*dz),
			((-d+z)*dy-(h+y)*dz)/(dy*dz))
	0<=min(
			-2*w/dx,
			-2*h/dy,
			-2*d/dz,
			(-w*dy-h*dx+x*dy-y*dx)/(dx*dy),
			(-w*dy-h*dx-x*dy+y*dx)/(dx*dy),
			(-d*dx-w*dz-z*dx+x*dz)/(dx*dz),
			(-d*dx-w*dz+z*dx-x*dz)/(dx*dz),
			(-h*dz-d*dy+y*dz-z*dy)/(dy*dz),
			(-h*dz-d*dy-y*dz+z*dy)/(dy*dz))
	0<=min(
			2*min(-w/dx,-h/dy,-d/dz),
			-w/dx-h/dy + min(x/dx-y/dy,-(x/dx-y/dy)),
			-d/dz-w/dx + min(z/dz-x/dx,-(z/dz-x/dx)),
			-h*dy-d*dz + min(y/dy-z/dz,-(y/dy-z/dz)))
	0<=min(
			2*min(-w/dx,-h/dy,-d/dz),
			-w/dx-h/dy - abs(x/dx-y/dy),
			-d/dz-w/dx - abs(z/dz-x/dx),
			-h*dy-d*dz - abs(y/dy-z/dz))
	*/

	if(numnodes==0){
		return false;
	}

	//#define longbits(x) (*(long long*)&(x))
	#if 0
	#define SWAP(x,y) tmpbool=(longbits(valarr[x].tlow)<longbits(valarr[y].tlow));\
						longbits(valarr[x].tlow)^=longbits(valarr[y].tlow);\
						valarr[x].index^=valarr[y].index;\
						\
						longbits(valarr[y].tlow)^= !tmpbool * longbits(valarr[x].tlow) /*if tmpbool is false then this stays the same*/;\
						valarr[y].index^= !tmpbool * valarr[x].index ;\
						\
						longbits(valarr[x].tlow)^=longbits(valarr[y].tlow);\
						valarr[x].index^=valarr[y].index;
	#else
	#if 0
	#define SWAP(x,y) tmpbool=(valarr[x].tlow>valarr[y].tlow);\
						valarr[x].tlow^=valarr[y].tlow;\
						valarr[x].index^=valarr[y].index;\
						\
						valarr[y].tlow^= tmpbool * valarr[x].tlow /*if tmpbool is false then this stays the same*/;\
						valarr[y].index^= tmpbool * valarr[x].index ;\
						\
						valarr[x].tlow^=valarr[y].tlow;\
						valarr[x].index^=valarr[y].index;
	#else
	#define SWAP(x,y) valarr[x].tlow^=valarr[y].tlow;\
						valarr[x].index^=valarr[y].index;\
						\
						valarr[y].index^= ((valarr[x].tlow^valarr[y].tlow) > valarr[y].tlow) * valarr[x].index ;\
						valarr[y].tlow^= ((valarr[x].tlow^valarr[y].tlow) > valarr[y].tlow) * valarr[x].tlow /*if tmpbool is false then this stays the same*/;\
						\
						valarr[x].tlow^=valarr[y].tlow;\
						valarr[x].index^=valarr[y].index;
	#endif
	#endif
	///generated via http://pages.ripco.net/~jgamble/nw.html
	SWAP(0,1);SWAP(2,3);SWAP(4,5);SWAP(6,7);
	SWAP(0,2);SWAP(1,3);SWAP(4,6);SWAP(5,7);
	SWAP(1,2);SWAP(5,6);SWAP(0,4);SWAP(3,7);
	SWAP(1,5);SWAP(2,6);
	SWAP(1,4);SWAP(3,6);SWAP(3,5);
	SWAP(2,4);
	SWAP(3,4);
	#undef SWAP
	#undef longbits

	++scale;

	for(int i=0;i<numnodes;i++){
		vec.x=varr[valarr[i].index].x*denom.d+p.x;
		vec.y=varr[valarr[i].index].y*denom.d+p.y;
		vec.z=varr[valarr[i].index].z*denom.d+p.z;

		tlow=valarr[i].tlow;

		if(chkIntersect(
				&(node->nodes->next[valarr[i].index]),
				vec,
				v,v0,pixrad,color,scale,(double*)&tlow
			)
		){
			*closeT=*(double*)&(valarr[i].tlow);
			return true;
		}
	}

	return false;
}
