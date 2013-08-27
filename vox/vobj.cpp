#include "vobj.h"
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace std;

#if 0
template<typename T>
T min(T a,T b){
	/*
	if(a<b){return a;}
	return b;
	/*/
	return (a<b)*a+(!(a<b))*b;
	//*/
}

template<typename T>
T max(T a,T b){
	/*
	if(a>b){return a;}
	return b;
	/*/
	return (a>b)*a+(!(a>b))*b;
	//*/
}
#endif

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

	#ifdef WIN64
	varr[0]=xvec;
	varr[1]=-xvec;
	varr[2]=yvec;
	varr[3]=-yvec;
	varr[4]=zvec;
	varr[5]=-zvec;
	#else
	varr={xvec,-xvec,yvec,-yvec,zvec,-zvec};
	#endif
}

bool vobj::intersects(vecref vec,vecref origin,const double pixrad,uint32_t* color,double* closeT) const{
	vec3d v0=origin;//copy the vector so that the original can be modified if this was called in parallel
	return chkIntersect(head,pos,pos,vec,v0,pixrad,color,1,closeT);
}
bool vobj::intersects(vecref vec,double x0,double y0,double z0,const double pixrad,uint32_t* color,double* closeT) const{
	vec3d v0(x0,y0,z0);
	return chkIntersect(head,pos,pos,vec,v0,pixrad,color,1,closeT);
}

typedef long long llong;

inline bool testRayCube(vecref p,vecref v,vecref v0,const double w,const double h,const double d,double& tlowRet,double& thighRet){
	#define x1 ( (1-2*(v.x>=0)) *w-(v0.x-p.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *w-(v0.x-p.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *h-(v0.y-p.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *h-(v0.y-p.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *d-(v0.z-p.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *d-(v0.z-p.z))/v.z

	const double
		tlow=max(max(x1,y1),z1),
		thigh=min(min(x2,y2),z2);

	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2

	tlowRet=tlow;
	thighRet=tlow;

	return thigh>=tlow;
}

#if 0
bool vobj::chkIntersect(vnode* node,vec3d p,vecref p0,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	vec3d vec;
	double tlow,thigh;
	double denom=0.5;
	int iclose;

	bool b=false;
	double tmp;
	uint32_t closeCol,tmpcol;

	double tmin=INFINITY;

	++scale;

	if(!(node->next)){
		if(testRayCube(p,v,v0,tlow,thigh)){
			*color=node->color;
			*closeT=max(tlow,0.0);
			return true;
		}
		return false;
	}

	while(true){
		b=false;
		denom/=2;

		///find which node is clo
		//i think this one is faster, need more testing to determine if it truely is or not
		uint32_t n=0;
		uint32_t nodes=0x00000000;

		#define blarg(i) \
		nodes|=(i*(((node->shape)>>i)&0x1))<<n;\
		n+=(((node->shape)>>i)&0x1)*3;

		blarg(0);blarg(1);blarg(2);blarg(3);
		blarg(4);blarg(5);blarg(6);blarg(7);
		//printf("%p\t%p\n",node->shape,nodes);

		#undef blarg

		//count the number of visible children nodes
		//01010101=0x55	00110011=0x33	00001111=0x0f
		n=((node->shape)&0x55)+(((node->shape)>>1)&0x55);
		n=(n&0x33)+((n>>2)&0x33);
		n=(n&0x0f)+((n>>4)&0x0f);

		int i=0;

		if(n>0){
			do{
				i=nodes&0x7;
				vec=varr[i&0x1];
				vec+=varr[2+((i>>1)&0x1)];
				vec+=varr[4+((i>>2)&0x1)];
				vec*=denom/2;
				vec+=p;

				if(testRayCube(vec,v,v0,w*denom,h*denom,d*denom,tlow,thigh))){
					b=true;

					iclose=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
					closeCol=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
					tmp=min(tmin,tmp);
				}
				nodes>>=3;
			}while(nodes!=0);
		}

	//if(tlow<=thigh){printf("%f %f\t",tlow,thigh);}

	///TODO: storing normals might be best
	#define norm_light 0

	while(true){
	tlow=max(tlow,0.0);

	//note that if thigh<0 implies tlow<0, if tlow<0 then tlow=0 because of the above, so this will include the check of thigh<0
	if(tlow>thigh){//the ray does not intersect the current node or the cube is completely behind the ray
		*closeT=-1;
		return false;
	}else if(node->next==NULL || node->shape==0x00 || pixrad*tlow*pixrad*tlow>w*w+h*h+d*d){///TODO: have a maxdepth //ray intersects and this is a leaf node
		*closeT=tlow;//max(tlow,0.0);
		#if norm_light==0
		*color=node->color;
		#else
		uint32_t r,g,b;
		//vec=vec3d((*closeT)*v.x+v0.x-p.x,(*closeT)*v.y+v0.y-p.y,(*closeT)*v.z+v0.z-p.z);
		//vec=vec3d((*closeT)*v.x+v0.x,(*closeT)*v.y+v0.y,(*closeT)*v.z+v0.z);
		vec=vec3d((*closeT)*v.x+v0.x-p0.x,(*closeT)*v.y+v0.y-p0.y,(*closeT)*v.z+v0.z-p0.z);

		const double x=abs(vec.x),y=abs(vec.y),z=abs(vec.z);
		float dot=abs(v.xyz[(y>x && y>z)+2*(z>x && z>y)]);

		r=(((node->color)>>16)&0xff)*dot;
		g=(((node->color)>>8)&0xff)*dot;
		b=((node->color)&0xff)*dot;
		*color=(r<<16)|(g<<8)|b;
		#endif
		return true;
	}

	//it intersects the current node, but the node is not a leaf node



	*color=closeCol;
	*closeT=tlow;
	return b;
}
#else
bool vobj::chkIntersect(vnode* node,vec3d p,vecref p0,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	//if(scale>1){printf("%i\n",scale);}
	const double denom=pow2(-scale);//1.0/(llong(1)<<llong(scale));// 1/(2^scale)

	#define x1 ( (1-2*(v.x>=0)) *w*denom-(v0.x-p.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *w*denom-(v0.x-p.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *h*denom-(v0.y-p.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *h*denom-(v0.y-p.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *d*denom-(v0.z-p.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *d*denom-(v0.z-p.z))/v.z

	double tlow=max(max(x1,y1),z1);
	const double thigh=min(min(x2,y2),z2);

	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2

	vec3d vec;

	//if(tlow<=thigh){printf("%f %f\t",tlow,thigh);}

	///TODO: storing normals might be best
	#define norm_light 0
	tlow=max(tlow,0.0);

	//note that if thigh<0 implies tlow<0, if tlow<0 then tlow=0 because of the above, so this will include the check of thigh<0
	if(tlow>thigh){//the ray does not intersect the current node or the cube is completely behind the ray
		*closeT=-1;
		return false;
	}else if(node->next==NULL || node->shape==0x00 || pixrad*tlow*pixrad*tlow>w*w+h*h+d*d){///TODO: have a maxdepth //ray intersects and this is a leaf node
		*closeT=tlow;//max(tlow,0.0);
		#if norm_light==0
		*color=node->color;
		#else
		uint32_t r,g,b;
		//vec=vec3d((*closeT)*v.x+v0.x-p.x,(*closeT)*v.y+v0.y-p.y,(*closeT)*v.z+v0.z-p.z);
		//vec=vec3d((*closeT)*v.x+v0.x,(*closeT)*v.y+v0.y,(*closeT)*v.z+v0.z);
		vec=vec3d((*closeT)*v.x+v0.x-p0.x,(*closeT)*v.y+v0.y-p0.y,(*closeT)*v.z+v0.z-p0.z);

		//*
		#if 0
		double maxdot=-2,tmpdot=-2;
		int maxi;
		vec3d vecarr[]={vec3d(1,0,0),vec3d(-1,0,0),vec3d(0,1,0),vec3d(0,-1,0),vec3d(0,0,1),vec3d(0,0,-1)};
		for(int i=0;i<6;i++){
			tmpdot=max(tmpdot,vec.dot(vecarr[i]));
			maxi=(tmpdot==maxdot)*maxi+(tmpdot!=maxdot)*i;
			maxdot=tmpdot;
		}
		//float dot=-v.dot(vecarr[maxi])*v.invMagnitude()*vecarr[maxi].invMagnitude();
		float dot=abs(v.dot(vecarr[maxi])*v.invMagnitude()*vecarr[maxi].invMagnitude());
		#else
		const double x=abs(vec.x),y=abs(vec.y),z=abs(vec.z);
		float dot=abs(v.xyz[(y>x && y>z)+2*(z>x && z>y)]);
		#endif
		/*/

		///This one looks kind of cool
		//vec.normalize();

		#define tol 0.5

		vec.x=sgn(vec.x)*(abs(vec.x)>=tol);
		vec.y=sgn(vec.y)*(abs(vec.y)>=tol);
		vec.z=sgn(vec.z)*(abs(vec.z)>=tol);

		#undef tol

		float dot=-v.dot(vec)*v.invMagnitude()*vec.invMagnitude();
		//*/

		//dot*=(dot>0);
		//dot=(dot+1)/2;
		//dot=1-(dot-1)*(dot-1);
		r=(((node->color)>>16)&0xff)*dot;
		g=(((node->color)>>8)&0xff)*dot;
		b=((node->color)&0xff)*dot;
		*color=(r<<16)|(g<<8)|b;
		#endif
		return true;
	}

	//it intersects the current node, but the node is not a leaf node
	bool b=false;
	double tmp;
	uint32_t closeCol,tmpcol;

	tlow=INFINITY;

	++scale;

	/*
	can i use tlow to figure out which node to go to next?
	could i use the same dot product method as i use with the camera?
	do the signs of those values actually tell me which quadrant to look in?
	i dont think that works because tlow relies on the axis aligned cube, which is too different form
	it might still work though
	it also would work if i were to figure out ray-parallelepiped intersection
	*/

	#if 1

	//i think this one is faster, need more testing to determine if it truely is or not

	uint32_t n=0;
	uint32_t nodes=0x00000000;

	#define blarg(i) \
	nodes|=(i*(((node->shape)>>i)&0x1))<<n;\
	n+=(((node->shape)>>i)&0x1)*3;

	blarg(0);
	blarg(1);
	blarg(2);
	blarg(3);
	blarg(4);
	blarg(5);
	blarg(6);
	blarg(7);

	//printf("%p\t%p\n",node->shape,nodes);

	#undef blarg

	//count the number of visible children nodes
	/*
	01010101=0x55
	00110011=0x33
	00001111=0x0f
	*/
	//*
	n=((node->shape)&0x55)+(((node->shape)>>1)&0x55);
	n=(n&0x33)+((n>>2)&0x33);
	n=(n&0x0f)+((n>>4)&0x0f);
	//*/

	int i;

	if(n>0){
		do{
			i=nodes&0x7;
			//if(chkIntersect(&(node->next[i]),p+denom/2*((1-((i<<1)&0x2))*xvec+(1-(i&0x2))*yvec+(1-((i>>1)&0x2))*zvec),v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
			vec=varr[i&0x1];
			vec+=varr[2+((i>>1)&0x1)];
			vec+=varr[4+((i>>2)&0x1)];
			vec*=denom/2;
			vec+=p;
			if(chkIntersect(&(node->next[i]),vec,p0,v,v0,pixrad,&tmpcol,scale,&tmp)){
				b=true;

				closeCol=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
				tlow=min(tlow,tmp);
			}
			nodes>>=3;
		}while(nodes!=0);
	}

	#else
	for(int i=0;i<8;i++){
		/*
		x=1-2*((i>>0)&0x1)=1-((i<<1)&0x2)
		y=1-2*((i>>1)&0x1)=1-(i&0x2)
		z=1-2*((i>>2)&0x1)=1-((i>>1)&0x2)
		*/
		//if the current child is supposed to exist
		if((node->shape>>i)&0x1){
			//if(chkIntersect(&(node->next[i]),p+denom/2*((1-((i<<1)&0x2))*xvec+(1-(i&0x2))*yvec+(1-((i>>1)&0x2))*zvec),v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
			vec=varr[i&0x1];
			vec+=varr[2+((i>>1)&0x1)];
			vec+=varr[4+((i>>2)&0x1)];
			vec*=denom/2;
			vec+=p;
			if(chkIntersect(&(node->next[i]),vec,v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
				b=true;

				closeCol=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
				tlow=min(tlow,tmp);
			}
		}
	}
	#endif

	*color=closeCol;
	*closeT=tlow;
	return b;
}
#endif



