#include "vobj.h"
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace std;

#if 0
template<typename T>
inline T min(T a,T b){
	/*
	if(a<b){return a;}
	return b;
	/*/
	return (a<b)*a+(!(a<b))*b;
	//*/
}

template<typename T>
inline T max(T a,T b){
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

typedef long long llong;

inline bool testRayCube(vecref p,vecref v,vecref v0,const double w,const double h,const double d,double& tlowRet,double& thighRet){
	#define x1 ( (1-2*(v.x>=0)) *w-(v0.x-p.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *w-(v0.x-p.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *h-(v0.y-p.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *h-(v0.y-p.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *d-(v0.z-p.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *d-(v0.z-p.z))/v.z

	tlowRet=max(max(x1,y1),z1);
	thighRet=min(min(x2,y2),z2);

	return thighRet>=tlowRet;

	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2
}
inline bool testRayCube(vecref p,vecref v,vecref v0,const double w,const double h,const double d,double& tlowRet){
	#define x1 ( (1-2*(v.x>=0)) *w-(v0.x-p.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *w-(v0.x-p.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *h-(v0.y-p.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *h-(v0.y-p.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *d-(v0.z-p.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *d-(v0.z-p.z))/v.z

	tlowRet=max(max(x1,y1),z1);
	return min(min(x2,y2),z2)>=tlowRet;

	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2
}
inline bool testRayCube(vecref p,vecref v,vecref v0,const double w,const double h,const double d){
	#define x1 ( (1-2*(v.x>=0)) *w-(v0.x-p.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *w-(v0.x-p.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *h-(v0.y-p.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *h-(v0.y-p.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *d-(v0.z-p.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *d-(v0.z-p.z))/v.z

	return min(min(x2,y2),z2)>=max(max(x1,y1),z1);

	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2
}

///TODO: try to make this w/o recursion
bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	//if(scale>1){printf("%i\n",scale);}
	vec3d vec;
	double denom=pow2(-scale);//1.0/(llong(1)<<llong(scale));// 1/(2^scale)

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

	///TODO: storing normals might be best
	#define norm_light 1
	tlow=max(tlow,0.0);

	//note that if thigh<0 implies tlow<0, if tlow<0 then tlow=0 because of the above line, so this will include the check of thigh<0
	#if 1
	///TODO: have a maxdepth
	if(tlow>thigh || !(node->next) || node->shape==0x00 || pixrad*tlow*pixrad*tlow>rsqr*denom*denom){//the ray does not intersect the current node or the cube is completely behind the ray
		//ray intersects and this is a leaf node


		#if norm_light==0
		#if 1
		(*color)^=(!(tlow>thigh))*((node->color) ^ (*color));
		// *color=node->color;
		#else
		#define maxdeep 9
		//const double pi=atan(1);
		//*color=255*cos(pi*(v.x*tlow+v0.x)/denom);
		//*color=255*cos(pi*(v.x*tlow+v0.x)*(0x1<<maxdeep));
		*color=255*((int)floor((v.x*tlow+v0.x)*(0x1<<maxdeep))%2);
		*color=*color**color/255;
		//*color=255*(scale)/maxdeep;
		*color=(*color<<16)|(*color<<8)|*color;
		#endif
		#else
		//if(!(tlow>thigh)){
			uint32_t r,g,b;

			#if 0
			const double
				x=abs((*closeT)*v.x+v0.x-p.x),
				y=abs((*closeT)*v.y+v0.y-p.y),
				z=abs((*closeT)*v.z+v0.z-p.z);
			#else
			const double
				x=abs(tlow*v.x+v0.x-p.x),
				y=abs(tlow*v.y+v0.y-p.y),
				z=abs(tlow*v.z+v0.z-p.z);
			#endif
			float dot=abs(v.xyz[(y>x && y>z)+2*(z>x && z>y)]);//*(pixrad*tlow*pixrad*tlow<=rsqr*denom*denom);
			//float dot=(abs(v.x)+abs(v.y)+abs(v.z))/3;

			//dot*=(dot>0);
			dot=(dot+1)/2;
			//dot=1-(dot-1)*(dot-1);
			r=(((node->color)>>16)&0xff)*dot;
			g=(((node->color)>>8)&0xff)*dot;
			b=((node->color)&0xff)*dot;
			*color^=(!(tlow>thigh))*(((r<<16)|(g<<8)|b)^(*color));
		//}
		#endif


		//*
		// *closeT=(!(tlow>thigh))*tlow + (tlow>thigh)*(*closeT);
		// *closeT=(!(tlow>thigh))?tlow:(*closeT);//max(tlow,0.0);
		*(long long*)closeT ^= (!(tlow>thigh))*((*(long long*)&tlow)^(*(long long*)closeT));
		/*/
		if(!(tlow>thigh)){
			*closeT=tlow;//max(tlow,0.0);
		}
		//*/

		return !(tlow>thigh);
	}
	#else
	if(tlow>thigh){//the ray does not intersect the current node or the cube is completely behind the ray
		return false;
	}else if(!(node->next) || node->shape==0x00 || pixrad*tlow*pixrad*tlow>rsqr*denom*denom){///TODO: have a maxdepth //ray intersects and this is a leaf node
		#if usecall
		*closeT=max(tlow,0.0);
		#else
		*closeT=tlow;//max(tlow,0.0);
		#endif

		#if norm_light==0
		#if 1
		*color=node->color;
		#else
		#define maxdeep 9
		//const double pi=atan(1);
		//*color=255*cos(pi*(v.x*tlow+v0.x)/denom);
		//*color=255*cos(pi*(v.x*tlow+v0.x)*(0x1<<maxdeep));
		*color=255*((int)floor((v.x*tlow+v0.x)*(0x1<<maxdeep))%2);
		*color=*color**color/255;
		//*color=255*(scale)/maxdeep;
		*color=(*color<<16)|(*color<<8)|*color;
		#endif
		#else
		uint32_t r,g,b;

		#if 0
		const double
			x=abs((*closeT)*v.x+v0.x-p.x),
			y=abs((*closeT)*v.y+v0.y-p.y),
			z=abs((*closeT)*v.z+v0.z-p.z);
		#else
		const double
			x=abs(tlow*v.x+v0.x-p.x),
			y=abs(tlow*v.y+v0.y-p.y),
			z=abs(tlow*v.z+v0.z-p.z);
		#endif
		float dot=abs(v.xyz[(y>x && y>z)+2*(z>x && z>y)]);//*(pixrad*tlow*pixrad*tlow<=rsqr*denom*denom);
		//float dot=(abs(v.x)+abs(v.y)+abs(v.z))/3;

		//dot*=(dot>0);
		dot=(dot+1)/2;
		//dot=1-(dot-1)*(dot-1);
		r=(((node->color)>>16)&0xff)*dot;
		g=(((node->color)>>8)&0xff)*dot;
		b=((node->color)&0xff)*dot;
		*color=(r<<16)|(g<<8)|b;
		#endif
		return true;
	}
	#endif

	//it intersects the current node, but the node is not a leaf node
	double tmp;
	bool b=false;
	uint32_t closeCol,tmpcol;

	tlow=INFINITY;
	++scale;
	denom/=2;

	for(int i=0;i<8;i++){
		#if 0
		/*
		vec=varr[i];
		vec*=denom;
		vec+=p;
		/*/
		vec=vec3d(
				varr[i].x*denom+p.x,
				varr[i].y*denom+p.y,
				varr[i].z*denom+p.z
		);
		//*/
		if((node->shape>>i)&0x1 && chkIntersect(&(node->next[i]),vec,v,v0,pixrad,&tmpcol,scale,&tmp)){
		//if((node->shape>>i)&0x1 && testRayCube(vec,v,v0,w*denom,h*denom,d*denom) && chkIntersect(&(node->next[i]),vec,p0,v,v0,pixrad,&tmpcol,scale,&tmp)){
		#else
		//*/
		if(
			(node->shape>>i)&0x1
			&&
			chkIntersect(
				&(node->next[i]),
				vec3d( varr[i].x*denom+p.x , varr[i].y*denom+p.y , varr[i].z*denom+p.z ),
				v,v0,pixrad,&tmpcol,scale,&tmp
			)
		){
		#endif
			b=true;

			closeCol=((tmp<tlow)*tmpcol)^((!(tmp<tlow))*closeCol);
			tlow=min(tlow,tmp);
		}
	}

	*color=closeCol;
	*closeT=tlow;
	return b;
}

#if 0///old, uncleaned version
bool vobj::chkIntersect(vnode* node,vec3d p,vecref p0,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	//if(scale>1){printf("%i\n",scale);}
	vec3d vec;
	double denom=pow2(-scale);//1.0/(llong(1)<<llong(scale));// 1/(2^scale)

	#define usecall 0

	#if !usecall
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

	//if(tlow<=thigh){printf("%f %f\t",tlow,thigh);}

	///TODO: storing normals might be best
	#define norm_light 0
	tlow=max(tlow,0.0);

	//note that if thigh<0 implies tlow<0, if tlow<0 then tlow=0 because of the above, so this will include the check of thigh<0
	if(tlow>thigh){//the ray does not intersect the current node or the cube is completely behind the ray
	#else
	double tlow;
	if(!testRayCube(p,v,v0,w*denom,h*denom,d*denom,tlow)){
	#endif
		return false;
	}else if(node->next==NULL || node->shape==0x00 || pixrad*tlow*pixrad*tlow>w*w+h*h+d*d){///TODO: have a maxdepth //ray intersects and this is a leaf node
		#if usecall
		*closeT=max(tlow,0.0);
		#else
		*closeT=tlow;//max(tlow,0.0);
		#endif

		#if norm_light==0
		*color=node->color;
		#else
		uint32_t r,g,b;
		//vec=vec3d((*closeT)*v.x+v0.x-p.x,(*closeT)*v.y+v0.y-p.y,(*closeT)*v.z+v0.z-p.z);
		//vec=vec3d((*closeT)*v.x+v0.x,(*closeT)*v.y+v0.y,(*closeT)*v.z+v0.z);
		vec=vec3d((*closeT)*v.x+v0.x-p0.x,(*closeT)*v.y+v0.y-p0.y,(*closeT)*v.z+v0.z-p0.z);

		//*
		const double x=abs(vec.x),y=abs(vec.y),z=abs(vec.z);
		float dot=abs(v.xyz[(y>x && y>z)+2*(z>x && z>y)]);
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
	double tmp;
	bool b=false;
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

	#if 0

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
	/*
	n=((node->shape)&0x55)+(((node->shape)>>1)&0x55);
	n=(n&0x33)+((n>>2)&0x33);
	n=(n&0x0f)+((n>>4)&0x0f);
	//*/

	int i;

	//if(n>0){
	if(node->shape){
		do{
			i=nodes&0x7;
			//if(chkIntersect(&(node->next[i]),p+denom/2*((1-((i<<1)&0x2))*xvec+(1-(i&0x2))*yvec+(1-((i>>1)&0x2))*zvec),v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
			vec=varr[i&0x1];
			vec+=varr[2+((i>>1)&0x1)];
			vec+=varr[4+((i>>2)&0x1)];
			vec*=denom/2;
			vec+=p;
			if(chkIntersect(&(node->next[i]),vec,p0,v,v0,pixrad,&tmpcol,scale,&tmp)){
			//if(testRayCube(vec,v,v0,w*denom,h*denom,d*denom) && chkIntersect(&(node->next[i]),vec,p0,v,v0,pixrad,&tmpcol,scale,&tmp)){
				b=true;

				closeCol=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
				tlow=min(tlow,tmp);
			}
			nodes>>=3;
		}while(nodes!=0);
	}

	#else
	denom/=2;
	for(int i=0;i<8;i++){
		/*
		x=1-2*((i>>0)&0x1)=1-((i<<1)&0x2)
		y=1-2*((i>>1)&0x1)=1-(i&0x2)
		z=1-2*((i>>2)&0x1)=1-((i>>1)&0x2)
		*/
		//if the current child is supposed to exist
		//if((node->shape>>i)&0x1){
			//if(chkIntersect(&(node->next[i]),p+denom/2*((1-((i<<1)&0x2))*xvec+(1-(i&0x2))*yvec+(1-((i>>1)&0x2))*zvec),v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
			vec=varr[i&0x1];
			vec+=varr[2+((i>>1)&0x1)];
			vec+=varr[4+((i>>2)&0x1)];
			vec*=denom;
			vec+=p;
			if((node->shape>>i)&0x1 && testRayCube(vec,v,v0,w*denom,h*denom,d*denom) && chkIntersect(&(node->next[i]),vec,p0,v,v0,pixrad,&tmpcol,scale,&tmp)){
				b=true;

				closeCol=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
				tlow=min(tlow,tmp);
			}
		//}
	}
	#endif

	*color=closeCol;
	*closeT=tlow;
	return b;
}
#endif
