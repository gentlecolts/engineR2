#include "vobj.h"
#if testmethod==2
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
using namespace std;

const int nextnode[]={
	//YZ,XZ,XY
	-1,-1,-1,//-1
	-1,-1,-1,//0
	0,-1,-1,//1
	-1,0,-1,//2
	2,1,-1,//3
	-1,-1,0,//4
	4,-1,1,//5
	-1,4,2,//6
	6,5,3//7
};

//constants, these do not change as the function iterates
struct constParams{
	double x0,y0,z0,dx,dy,dz;//transformed ray vector and position
	int sdx,sdy,sdz;//constants for flip
	int flip;
};
//these values do not need to be re-allocated, so they can be re-used
struct reusable{
	union{double d;long long l;} tx[3],ty[3],tz[3],tmin,tmax;
};

#define moreIfs 0
bool recurse(vnode* node,const double px,const double py,const double pz,reusable& vals,const constParams& ray,const double scale,uint32_t *color){
	struct {long long t;int node;} order[5];
	const double
		x=ray.x0-px,
		y=ray.y0-py,
		z=ray.z0-pz;
	/*
	#define longbits(x) (*(long long*)&(x))
	#define ltmin longbits(vals.tmin)
	#define ltmax longbits(vals.tmax)
	//*/

	//calulate the plane intersections
	//note that the order of these arrays was chosen to minimize index computation operations
	vals.tx[0].d=(scale-x)/ray.dx;vals.tx[1].d=-x/ray.dx;vals.tx[2].d=(-scale-x)/ray.dx;
	vals.ty[0].d=(scale-y)/ray.dy;vals.ty[1].d=-y/ray.dy;vals.ty[2].d=(-scale-y)/ray.dy;
	vals.tz[0].d=(scale-z)/ray.dz;vals.tz[1].d=-z/ray.dz;vals.tz[2].d=(-scale-z)/ray.dz;

	#if moreIfs
	//these can be computed and stored to vals before calling the function
	vals.tmin.l=max(max(vals.tx[2].l,vals.ty[2].l),vals.tz[2].l);
	vals.tmax.l=min(min(vals.tx[0].l,vals.ty[0].l),vals.tz[0].l);

	if((vals.tmin.l>vals.tmax.l) | (vals.tmax.l<0)){//misses node
		return false;
	}
	#endif

	///if the above conditional can be skipped, then this goes first
	if((!node->nodes) | (!node->shape)){
	//if(((!node->nodes) | (!node->shape)) & (vals.tmax.l>0)){
		*color=node->color;
		return true;
	}

	//find starting node
	order[0].t=vals.tmin.l;
	order[0].node=((vals.tmin.l<=vals.tz[1].l)<<2)|((vals.tmin.l<=vals.ty[1].l)<<1)|(vals.tmin.l<=vals.tx[1].l);

	//find the rest of the nodes
	#define YZ(n) (vals.tx[(order[n].node)&0x1].l)
	#define XZ(n) (vals.ty[((order[n].node)&0x2)>>1].l)
	#define XY(n) (vals.tz[((order[n].node)&0x4)>>2].l)
	#define nodecomp(n) (3*(order[n].node+1)+\
		(((XZ(n)<YZ(n)) & (XZ(n)<=XY(n))) | \
		(((XY(n)<YZ(n)) & (XY(n)<XZ(n)))<<1)))
	#define tval(n) (min(min(XY(n),XZ(n)),YZ(n)))

	order[1].node=nextnode[nodecomp(0)];
	order[1].t=tval(0);
	order[2].node=nextnode[nodecomp(1)];
	order[2].t=tval(1);
	order[3].node=nextnode[nodecomp(2)];
	order[3].t=tval(2);
	order[4].node=-1;//this last one is for ensuring all intersections are accounted for, the ray will never traverse a 5th node
	order[4].t=tval(3);

	#define counting 0
	#if counting
	const int len=(order[0].node!=-1)+(order[1].node!=-1)+(order[2].node!=-1)+(order[3].node!=-1);
	#endif

	int inext;
	vnode *next;
	#if counting
	for(int i=0;i<len;i++){
	#else
	//for(int i=0;i<4;i++){
	int i=0;
	while(order[i].node!=-1){
	#endif
		inext=order[i].node^ray.flip;
		next=&(node->nodes->next[inext]);
		//if(order[i].node==-1){return false;}
		#if !moreIfs
		vals.tmin.l=order[i].t;
		vals.tmax.l=order[i+1].t;
		#endif
		if(
			(((node->shape>>inext)&0x1) & (order[i+1].t>0))
			//((node->shape>>inext)&0x1)
			&&
			recurse(
				next,
				px+scale*(0.5-(order[i].node&0x1)),
				py+scale*(0.5-((order[i].node>>1)&0x1)),
				pz+scale*(0.5-((order[i].node>>2)&0x1)),
				vals,ray,scale*0.5,color
			)
		){
			return true;
		}

		++i;
	}
	return false;
}

//set up the constants and  then call the recursive function
bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	constParams ray;
	//calculate the transformed direction vectors
	ray.dx=(invmtr[0]*v.x+invmtr[1]*v.y+invmtr[2]*v.z)*invdet;
	ray.dy=(invmtr[3]*v.x+invmtr[4]*v.y+invmtr[5]*v.z)*invdet;
	ray.dz=(invmtr[6]*v.x+invmtr[7]*v.y+invmtr[8]*v.z)*invdet;
	ray.x0=(invmtr[0]*(v0.x-p.x)+invmtr[1]*(v0.y-p.y)+invmtr[2]*(v0.z-p.z))*invdet;
	ray.y0=(invmtr[3]*(v0.x-p.x)+invmtr[4]*(v0.y-p.y)+invmtr[5]*(v0.z-p.z))*invdet;
	ray.z0=(invmtr[6]*(v0.x-p.x)+invmtr[7]*(v0.y-p.y)+invmtr[8]*(v0.z-p.z))*invdet;
	//get the signs and compute the constant for flipping with xor
	ray.sdx=sgn(ray.dx);
	ray.sdy=sgn(ray.dy);
	ray.sdz=sgn(ray.dz);
	ray.flip=((ray.sdz<0)<<2)|((ray.sdy<0)<<1)|(ray.sdx<0);

	//flip the origin and direction
	ray.x0*=ray.sdx;
	ray.dx*=ray.sdx;
	ray.y0*=ray.sdy;
	ray.dy*=ray.sdy;
	ray.z0*=ray.sdz;
	ray.dz*=ray.sdz;

	reusable vals;

	#if !moreIfs
	vals.tx[0].d=(1-ray.x0)/ray.dx;vals.tx[2].d=(-1-ray.x0)/ray.dx;
	vals.ty[0].d=(1-ray.y0)/ray.dy;vals.ty[2].d=(-1-ray.y0)/ray.dy;
	vals.tz[0].d=(1-ray.z0)/ray.dz;vals.tz[2].d=(-1-ray.z0)/ray.dz;
	vals.tmin.l=max(max(vals.tx[2].l,vals.ty[2].l),vals.tz[2].l);
	vals.tmax.l=min(min(vals.tx[0].l,vals.ty[0].l),vals.tz[0].l);

	//*
	if((vals.tmin.l>vals.tmax.l) | (vals.tmax.l<0)){//misses node
		return false;
	}
	//*/
	#endif

	//return recurse(node,p.x,p.y,p.z,vals,ray,1,color);
	return recurse(node,0,0,0,vals,ray,1,color);
}
#endif
