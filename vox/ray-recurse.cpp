#include "vobj.h"
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
using namespace std;

/*this table is used for determining what the next node on the ray's path is
the value is based on the current node and the plane which the ray exits the node from
note that it may also be possible to create this with clever use of xor and bitshifts
-however dealing with the case of having exited the node (currently represented by -1) becomes more complicated
-as such, this table is used instead
--the table is probably cached well enough that the performance difference is negledgeable
--performance is maybe even better with the table than with computing the value
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

//constants, these do not change as the function iterates
struct constParams{
	double x0,y0,z0,dx,dy,dz;//transformed ray vector and position
	int sdx,sdy,sdz;//constants for flip
	int flip;
};
//these values do not need to be re-allocated, so they can be re-used
struct reusable{
	/*the reason for this union is because:
	-treating the double as a long long does not impact the result of comparisons
	-double comparisons have to deal with handling inf
	--this worsens performance AND produces undesireable behavior
	*/
	union{double d;long long l;} tx[3],ty[3],tz[3],tmin,tmax;
};

bool recurse(
			vnode* node,
			const double px,const double py,const double pz,
			reusable& vals,const constParams& ray,
			const double scale,
			uint32_t *color,double* closeT
){
	/*
	if we've reached a leaf node, then we can return true
	we can do this because by this point we KNOW that the ray intersects the node and that the node exists

	when incremental loading is implemented it will look something like:
	if((!node->nodes) | (!node->shape)){
		if((!node->nodes)&(node->shape){//the next nodes do not exist, but the shape says they should
			//queue the next part of the tree to be loaded
		}
		//determine the color, distance, etc and return true
	}

	also note: instead of loading nodes based on being needed as shown above,
	it may be better to load based on distance from the camera, impact on light rays, etc
	*/
	if((!node->nodes) | (!node->shape)){
		/**TODO: implement a more sophisticated means of determining color
		this will include giving the nodes material properties
		i also plan to use photon mapping, so that will be a part of it
		*/
		*color=node->color;
		*(long long*)closeT=max(vals.tmin.l,0ll);
		return true;
	}

	struct {long long t;int node;} order[5];//will be used to store the path which the ray traverses the node
	const double
		x=ray.x0-px,
		y=ray.y0-py,
		z=ray.z0-pz;
	/*calulate the plane intersections
	note that the order of these arrays was chosen to minimize index computation operations
	this is because the possible exit planes of the child nodes are stored at index 0 and 1 for each t,
	meaning a boolean is all that is needed to determine which of the possibilities to choose
	*/
	vals.tx[0].d=(scale-x)/ray.dx;vals.tx[1].d=-x/ray.dx;vals.tx[2].d=(-scale-x)/ray.dx;
	vals.ty[0].d=(scale-y)/ray.dy;vals.ty[1].d=-y/ray.dy;vals.ty[2].d=(-scale-y)/ray.dy;
	vals.tz[0].d=(scale-z)/ray.dz;vals.tz[1].d=-z/ray.dz;vals.tz[2].d=(-scale-z)/ray.dz;

	//find starting node
	order[0].t=vals.tmin.l;
	order[0].node=((vals.tmin.l<=vals.tz[1].l)<<2)|((vals.tmin.l<=vals.ty[1].l)<<1)|(vals.tmin.l<=vals.tx[1].l);

	//find the rest of the nodes
	//these first three macros are used to determine the ts which represent the exit planes along each axis
	#define YZ(n) (vals.tx[(order[n].node)&0x1].l)
	#define XZ(n) (vals.ty[((order[n].node)&0x2)>>1].l)
	#define XY(n) (vals.tz[((order[n].node)&0x4)>>2].l)
	//this determines the index in nextnode that contains the correct next node along the path
	#define nodecomp(n) (3*(order[n].node+1)+\
		(((XZ(n)<YZ(n)) & (XZ(n)<=XY(n))) | \
		(((XY(n)<YZ(n)) & (XY(n)<XZ(n)))<<1)))
	//this is the t value for which the ray enters the current node
	#define tval(n) (min(min(XY(n),XZ(n)),YZ(n)))

	order[1].node=nextnode[nodecomp(0)];
	order[1].t=tval(0);
	order[2].node=nextnode[nodecomp(1)];
	order[2].t=tval(1);
	order[3].node=nextnode[nodecomp(2)];
	order[3].t=tval(2);
	order[4].node=-1;//this last one is for ensuring all intersections are accounted for, the ray will never traverse a 5th node
	order[4].t=tval(3);

	///TODO: test if storing inext or not has a significant performance impact
	#define storenext 1
	#if storenext
	int inext,i=0;
	#else
	#define inext (order[i].node^ray.flip)
	int i=0;
	#endif
	while(order[i].node!=-1){
		#if storenext
		inext=order[i].node^ray.flip;
		#endif
		vals.tmin.l=order[i].t;
		vals.tmax.l=order[i+1].t;

		if(
			(((node->shape>>inext)&0x1) & (order[i+1].t>0))//does the node exist and is in front of the camera
			&&
			recurse(
				&(node->nodes->next[inext]),
				px+scale*(0.5-(order[i].node&0x1)),//note that these do not need to be flipped
				py+scale*(0.5-((order[i].node>>1)&0x1)),//this is because these coordinates are in the transformed coordinate system
				pz+scale*(0.5-((order[i].node>>2)&0x1)),//therefore the transform does not need to be undone by flipping
				vals,ray,scale*0.5,color,closeT
			)
		){
			return true;
		}

		++i;
	}
	return false;
}

/**TODO: make this all a bit more streamlined, i probably dont need this series of function calls
also, re-work the paramaters of these functions, there are plenty of unnecessary/condensable parameters
*/
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

	//determine the values for intersection between the ray and the whole cube
	vals.tx[0].d=(1-ray.x0)/ray.dx;vals.tx[2].d=(-1-ray.x0)/ray.dx;
	vals.ty[0].d=(1-ray.y0)/ray.dy;vals.ty[2].d=(-1-ray.y0)/ray.dy;
	vals.tz[0].d=(1-ray.z0)/ray.dz;vals.tz[2].d=(-1-ray.z0)/ray.dz;
	vals.tmin.l=max(max(vals.tx[2].l,vals.ty[2].l),vals.tz[2].l);
	vals.tmax.l=min(min(vals.tx[0].l,vals.ty[0].l),vals.tz[0].l);

	/*check if the ray can even hit the object
	note that this only needs to be done once, once we know the ray hits,
	we then know that any nodes along the paths we generate will be hit by the ray
	*/
	if((vals.tmin.l>vals.tmax.l) | (vals.tmax.l<0)){
		return false;
	}

	//return recurse(node,p.x,p.y,p.z,vals,ray,1,color);
	return recurse(node,0,0,0,vals,ray,1,color,closeT);
}
