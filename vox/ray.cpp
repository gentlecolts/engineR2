#include "vobj.h"
#if testmethod==1
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace std;

/**
this file is for handling ray operations
*/

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

struct params{
	struct {long long t;int node;} order[5];
	double x,y,z,scale;//origin and scale of current node
	int i;
	params *prev;//,*next;//next is only needed if the chosen algorithm only deallocates at the end (see following todo)
	vnode *node;

	params(vnode* nodepntr):x(0),y(0),z(0),scale(1),i(0),prev(0),node(nodepntr){}
	params(vnode* nodepntr,params* prevpntr,double x0,double y0,double z0,double s=1):
		x(x0),y(y0),z(z0),scale(s),
		i(0),
		prev(prevpntr),node(nodepntr)
	{}
	params(params *current,const int flip):
		x(current->x+current->scale*(0.5-(current->order[current->i].node&0x1))),
		y(current->y+current->scale*(0.5-((current->order[current->i].node>>1)&0x1))),
		z(current->z+current->scale*(0.5-((current->order[current->i].node>>2)&0x1))),
		scale(current->scale*0.5),///TODO: determin which of *0.5 or /2 is better here
		i(0),
		prev(current),
		node(&(current->node->nodes->next[current->order[current->i].node^flip]))
	{}
};

/**TODO:
-compare performace of:
  doing allocations and deallocations every iteration
  using conditional branches to save deallocations till the end
-create a version of this function that uses an array (will require maxdepth)
*/

union lldouble{
	double d;
	long long l;
};

bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	//initialize

	//store transformed direction vector
	double
		dx=(invmtr[0]*v.x+invmtr[1]*v.y+invmtr[2]*v.z)*invdet,
		dy=(invmtr[3]*v.x+invmtr[4]*v.y+invmtr[5]*v.z)*invdet,
		dz=(invmtr[6]*v.x+invmtr[7]*v.y+invmtr[8]*v.z)*invdet;
	double x,y,z;
	lldouble tx[3],ty[3],tz[3],tmin,tmax;
	params *current=new params(node),*tmppntr;
	const int sdx=sgn(dx),sdy=sgn(dy),sdz=sgn(dz);
	const int flip=((sdz<0)<<2)|((sdy<0)<<1)|(sdx<0);

	//transform coordinates based on the component vectors and flip signs appropriately
	const double
		x0=(invmtr[0]*(v0.x-p.x)+invmtr[1]*(v0.y-p.y)+invmtr[2]*(v0.z-p.z))*invdet*sdx,
		y0=(invmtr[3]*(v0.x-p.x)+invmtr[4]*(v0.y-p.y)+invmtr[5]*(v0.z-p.z))*invdet*sdy,
		z0=(invmtr[6]*(v0.x-p.x)+invmtr[7]*(v0.y-p.y)+invmtr[8]*(v0.z-p.z))*invdet*sdz;

	dx*=sdx;
	dy*=sdy;
	dz*=sdz;

	/*
	#define longbits(x) (*(long long*)&(x))
	#define ltmin longbits(tmin)
	#define ltmax longbits(tmax)
	//*/

	#if 1
	//check if we hit the root node
	x=x0-current->x;
	y=y0-current->y;
	z=z0-current->z;

	//calulate the plane intersections
	//note that the order of these arrays was chosen to minimize index computation operations
	tx[0].d=(current->scale-x)/dx;tx[1].d=-x/dx;tx[2].d=(-current->scale-x)/dx;
	ty[0].d=(current->scale-y)/dy;ty[1].d=-y/dy;ty[2].d=(-current->scale-y)/dy;
	tz[0].d=(current->scale-z)/dz;tz[1].d=-z/dz;tz[2].d=(-current->scale-z)/dz;
	tmin.l=max(max(tx[2].l,ty[2].l),tz[2].l);
	tmax.l=min(min(tx[0].l,ty[0].l),tz[0].l);

	//if our ray hits the root node, then it will hit everything along the path we determine
	//this check does not need to be made every iteration
	if((tmin.l>tmax.l) | (tmax.l<0)){
		return false;
	}
	//goto firstloop;

	//begin loop
	loop:
		//generate ordered list of intersecting nodes
		//set up values

		x=x0-current->x;
		y=y0-current->y;
		z=z0-current->z;

		//calulate the plane intersections
		//note that the order of these arrays was chosen to minimize index computation operations
		tx[0].d=(current->scale-x)/dx;tx[1].d=-x/dx;tx[2].d=(-current->scale-x)/dx;
		ty[0].d=(current->scale-y)/dy;ty[1].d=-y/dy;ty[2].d=(-current->scale-y)/dy;
		tz[0].d=(current->scale-z)/dz;tz[1].d=-z/dz;tz[2].d=(-current->scale-z)/dz;
		tmin.l=max(max(tx[2].l,ty[2].l),tz[2].l);
		//ltmax=tmax.l=min(min(tx[0].l,ty[0].l),tz[0].l);

		//firstloop:
	#else
	loop:
		//generate ordered list of intersecting nodes
		//set up values

		x=x0-current->x;
		y=y0-current->y;
		z=z0-current->z;

		//calulate the plane intersections
		//note that the order of these arrays was chosen to minimize index computation operations
		tx[0].d=(current->scale-x)/dx;tx[1].d=-x/dx;tx[2].d=(-current->scale-x)/dx;
		ty[0].d=(current->scale-y)/dy;ty[1].d=-y/dy;ty[2].d=(-current->scale-y)/dy;
		tz[0].d=(current->scale-z)/dz;tz[1].d=-z/dz;tz[2].d=(-current->scale-z)/dz;
		tmin.l=max(max(tx[2].l,ty[2].l),tz[2].l);
		tmax.l=min(min(tx[0].l,ty[0].l),tz[0].l);

		if((tmin.l>tmax.l) | (tmax.l<0)){
			goto up;
		}
	#endif

		//find starting node
		current->order[0].t=tmin.l;
		current->order[0].node=((tmin.l<=tz[1].l)<<2)|((tmin.l<=ty[1].l)<<1)|(tmin.l<=tx[1].l);

		//find the rest of the nodes
		#define YZ(n) (tx[(current->order[n].node)&0x1].l)
		#define XZ(n) (ty[((current->order[n].node)&0x2)>>1].l)
		#define XY(n) (tz[((current->order[n].node)&0x4)>>2].l)
		#define nodecomp(n) (3*(current->order[n].node+1)+\
			(((XZ(n)<YZ(n)) & (XZ(n)<=XY(n))) | \
			(((XY(n)<YZ(n)) & (XY(n)<XZ(n)))<<1)))
		#define tval(n) (min(min(XY(n),XZ(n)),YZ(n)))

		current->order[1].node=nextnode[nodecomp(0)];
		current->order[1].t=tval(0);
		current->order[2].node=nextnode[nodecomp(1)];
		current->order[2].t=tval(1);
		current->order[3].node=nextnode[nodecomp(2)];
		current->order[3].t=tval(2);
		current->order[4].node=-1;//this last one is for ensuring all intersections are accounted for, the ray will never traverse a 5th node
		current->order[4].t=tval(3);

		/*
		//flip value of nodes
		current->order[0].node^=flip;
		current->order[1].node^=flip;
		current->order[2].node^=flip;
		current->order[3].node^=flip;
		//*/

		///TODO: comment this...please...
		down:
		while((current->order[current->i+1].t<0)|!((current->node->shape>>(current->order[current->i].node^flip))&0x1)){
			++current->i;
			if(current->order[current->i].node==-1){//note that if current->i>3, then order.node will always be -1
				goto up;
			}
		}

		current=new params(current,flip);
		if(!(current->node->shape)|!(current->node->nodes)){
			*color=current->node->color;
			*((long long*)closeT)=current->prev->order[current->prev->i].t;

			while(current->prev){
				tmppntr=current;
				current=current->prev;
				delete tmppntr;
			}
			delete current;

			return true;
		}
		goto loop;

		up:
			#if 0
			do{
				//*
				if(!(current->prev)){//the ray has missed the root node
					delete current;
					return false;
				}
				tmppntr=current;
				current=current->prev;
				delete tmppntr;
				/*/
				//this order of operations allows the branch to be determined well before it is reached, which may have better performance
				///TODO: benchmark
				//current results indicate that the previous method was in fact faster
				const bool done=!(current->prev);
				tmppntr=current;
				current=current->prev;
				delete tmppntr;
				if(done){return false;}
				//*/
			}while((current->i==3)|(current->order[current->i].node==-1));//if we've missed all of the childern of the node, or have exited it, then go up again
			#else
			//if(!current){printf("current was null");exit(1);}
			do{
				tmppntr=current;
				current=current->prev;
				delete tmppntr;
			}while(current&&((current->i==3)|(current->order[current->i].node==-1)));
			if(!current){return false;}
			#endif

			++current->i;
		goto down;
}

/*
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
		float dot=abs(v.xyz[((*(long long*)&y>*(long long*)&x) & (*(long long*)&y>*(long long*)&z))|(((*(long long*)&z>*(long long*)&x) & (*(long long*)&z>*(long long*)&y))<<1)]);//*(pixrad*tlow*pixrad*tlow<=rsqr*denom*denom);
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
*/
#endif
