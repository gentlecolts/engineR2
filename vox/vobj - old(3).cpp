#if 0
#include "vobj - old(2).cpp"
#else
#include "vobj.h"
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>

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

typedef uint32_t dtype;//depth type

struct gridcoord{
	dtype x,y,z;
};

struct nodebox{
	dtype x,y,z,depth,maxdepth;
	vnode* node;
};

inline bool nodecheck(nodebox& pos){
	int nextchild;
	bool b;

	pos.depth=pos.maxdepth-1;

	test:
		//nextchild=(((pos.z>>pos.depth)&0x1)<<2)|(((pos.y>>pos.depth)&0x1)<<1)|((pos.x>>pos.depth)&0x1);
		nextchild=(((pos.z>>pos.depth)&0x1)<<2)|(((pos.y>>pos.depth)&0x1)<<1)|((pos.x>>pos.depth)&0x1);

		b=(!(pos.node->shape) | !(pos.node->nodes) | pos.depth==0);

		if(b || !((pos.node->shape>>nextchild)&0x1)){
			return b;
		}

		pos.node=&(pos.node->nodes->next[nextchild]);
		--pos.depth;
	goto test;
}

inline bool nodecheck(nodebox& pos,const gridcoord& pnt){
	int nextchild;
	dtype depth;
	bool b;

	while(
			(pnt.x>>(pos.maxdepth-pos.depth))!=pos.x |
			(pnt.y>>(pos.maxdepth-pos.depth))!=pos.y |
			(pnt.z>>(pos.maxdepth-pos.depth))!=pos.z
	){
		pos.x>>=1;
		pos.y>>=1;
		pos.z>>=1;
		--pos.depth;
		pos.node=pos.node->getParent();
	}

	depth=pos.maxdepth-1;

	test:
		//nextchild=(((pos.z>>pos.depth)&0x1)<<2)|(((pos.y>>pos.depth)&0x1)<<1)|((pos.x>>pos.depth)&0x1);
		nextchild=(((pos.z>>depth)&0x1)<<2)|(((pos.y>>depth)&0x1)<<1)|((pos.x>>depth)&0x1);

		b=(!(pos.node->shape) | !(pos.node->nodes) | depth==0);

		if(b || !((pos.node->shape>>nextchild)&0x1)){
			pos.x=pnt.x>>(pos.maxdepth-depth-1);
			pos.y=pnt.y>>(pos.maxdepth-depth-1);
			pos.z=pnt.z>>(pos.maxdepth-depth-1);
			pos.depth=pos.maxdepth-depth-1;
			return b;
		}

		pos.node=&(pos.node->nodes->next[nextchild]);
		--pos.depth;
	goto test;
}

#define logging 0
#define log0 1
#define log1 1
#define log2 1
#define log3 1

bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	vec3d
	//*
		p0(
			(v0.x-p.x)/w,
			(v0.y-p.y)/h,
			(v0.z-p.z)/d),
		delta(v.x/w,v.y/h,v.z/d);
	//delta.normalize();
	/*/
		p0(
			((v0.x-p.x)/w+1)/2,
			((v0.y-p.y)/h+1)/2,
			((v0.z-p.z)/d+1)/2),
		delta(v.x/w/2,v.y/h/2,v.z/d/2);
	//*/

	///initialize stringstream and record the initial vector and box position
	#if logging
	char numstr[65];
	stringstream sstr;

	sstr<<"---BEGIN TEST---"<<endl;

	#if log0
	sstr<<"object pos: ( "<<p.x<<" , "<<p.y<<" , "<<p.z<<" )"<<endl;
	sstr<<"v0: ( "<<v0.x<<" , "<<v0.y<<" , "<<v0.z<<" )"<<endl;
	sstr<<"v: ( "<<v.x<<" , "<<v.y<<" , "<<v.z<<" )"<<endl;
	#endif
	#endif

	const double
		x1=( (1-2*(v.x>=0)) -p0.x)/delta.x,
		x2=( (2*(v.x>=0)-1) -p0.x)/delta.x,
		y1=( (1-2*(v.y>=0)) -p0.y)/delta.y,
		y2=( (2*(v.y>=0)-1) -p0.y)/delta.y,
		z1=( (1-2*(v.z>=0)) -p0.z)/delta.z,
		z2=( (2*(v.z>=0)-1) -p0.z)/delta.z;

	#define longbits(x) (*(long long*)&(x))

	double tlow,thigh;
	longbits(tlow)=max(max(max(longbits(x1),longbits(y1)),longbits(z1)),0ll);
	longbits(thigh)=min(min(longbits(x2),longbits(y2)),longbits(z2));
	if(tlow>thigh){
	//if(tlow>thigh | thigh-tlow>1){
		return false;
	}

	#if 0
	{
		const double
			x3=tlow*delta.x+p0.x,
			y3=tlow*delta.y+p0.y,
			z3=tlow*delta.z+p0.z,
			x4=thigh*delta.x+p0.x,
			y4=thigh*delta.y+p0.y,
			z4=thigh*delta.z+p0.z;
		#define bottom -1
		if(
			x3<bottom || x3>1 ||
			y3<bottom || y3>1 ||
			z3<bottom || z3>1 ||
			x4<bottom || x4>1 ||
			y4<bottom || y4>1 ||
			z4<bottom || z4>1
		){
			return false;
		}
	}
	#endif

	///record tlow, thigh, and the intersection point
	#if logging && log1
	sstr<<"tlow: "<<tlow<<endl;
	sstr<<"intersect: ( "<<p0.x+tlow*delta.x<<" , "<<p0.y+tlow*delta.y<<" , "<<p0.z+tlow*delta.z<<" )"<<endl;
	sstr<<"thigh: "<<thigh<<endl;
	sstr<<"intersect: ( "<<p0.x+thigh*delta.x<<" , "<<p0.y+thigh*delta.y<<" , "<<p0.z+thigh*delta.z<<" )"<<endl;
	#endif

	//const double gsize=pow2(maxdepth)-1;
	const dtype gsize=(0x1ll<<maxdepth);

	//*
	const dtype len=max(max(abs(delta.x),abs(delta.y)),abs(delta.z))*abs(tlow-thigh)*gsize+1;
	/*/
	const dtype len=3*gsize+1;
	//*/

	///record gridsize, dt, and len
	#if logging && log2
	sstr<<"delta: ( "<<delta.x<<" , "<<delta.y<<" , "<<delta.z<<" )"<<endl;
	sstr<<"gridsize: "<<gsize<<endl;
	sstr<<"dt: "<<dt<<endl;
	sstr<<"len: "<<len<<endl;
	#endif

	#if 1
	p0.x=(p0.x+tlow*delta.x+1)*0.5*gsize;
	p0.y=(p0.y+tlow*delta.y+1)*0.5*gsize;
	p0.z=(p0.z+tlow*delta.z+1)*0.5*gsize;

	/*
	const double dt=(thigh-tlow)/len;
	delta*=dt*0.5*gsize;
	/*/
	delta*=(thigh-tlow)/(2*len)*gsize;
	//*/
#else
	p0.x=(p0.x+tlow*delta.x)*gsize;
	p0.y=(p0.y+tlow*delta.y)*gsize;
	p0.z=(p0.z+tlow*delta.z)*gsize;

	/*
	const double dt=(thigh-tlow)/len;
	delta*=dt*0.5*gsize;
	/*/
	delta*=(thigh-tlow)/len*gsize;
	//*/
	#endif

	#define makecall 0
	#define go_up 0

	#if !makecall
	int nextchild;
	bool b;
	#endif

	nodebox gpos;
	gpos.maxdepth=maxdepth;

	#if go_up
	gpos.x=0;
	gpos.y=0;
	gpos.z=0;
	gpos.depth=0;
	gpos.node=node;

	gridcoord pnt;
	#endif

	for(dtype i=0;i<=len;i++){
		#if go_up
		pnt.x=1-p0.x-i*delta.x;
		pnt.y=1-p0.y-i*delta.y;
		pnt.z=1-p0.z-i*delta.z;
		#else
		//*
		gpos.x=gsize-(p0.x+i*delta.x);
		gpos.y=gsize-(p0.y+i*delta.y);
		gpos.z=gsize-(p0.z+i*delta.z);
		/*/
		gpos.x=1-p0.x-i*delta.x;
		gpos.y=1-p0.y-i*delta.y;
		gpos.z=1-p0.z-i*delta.z;
		//*/

		gpos.node=node;

		gpos.depth=maxdepth-1;
		#endif

		///record i and gpos
		#if logging && log3
		sstr<<"i: "<<i<<endl;
		sstr<<"x: "<<itoa(gpos.x,numstr,2)<<endl;
		sstr<<"y: "<<itoa(gpos.y,numstr,2)<<endl;
		sstr<<"z: "<<itoa(gpos.z,numstr,2)<<endl;
		#endif

		#if makecall
		#if go_up
		if(nodecheck(gpos,pnt)){
		#else
		if(nodecheck(gpos)){
		#endif
			///record hit
			#if logging
			sstr<<endl;
			printf(sstr.str().c_str());
			#endif
			*color=gpos.node->color;
			return true;
		}
		#else

		//gpos.depth=gpos.maxdepth-1;

		#if 0
		b=true;
		//for(int j=0;j<maxdepth;j++){
		//for(int j=0;j<maxdepth & b;j++){
		//for(;b;){
		while(b){
			nextchild=(((gpos.z>>gpos.depth)&0x1)<<2)|(((gpos.y>>gpos.depth)&0x1)<<1)|((gpos.x>>gpos.depth)&0x1);

			b=!(!(gpos.node->shape) | !(gpos.node->nodes) | gpos.depth==0 | !((gpos.node->shape>>nextchild)&0x1)) ;

			gpos.node=(vnode*)
				(((long long)(int*)(gpos.node))^(b*(((long long)(int*)&(gpos.node->nodes->next[nextchild]))^(long long)(int*)(gpos.node))));
			gpos.depth-=b;
		}

		//if(!(gpos.node->shape) | !(gpos.node->nodes) | gpos.depth==0){
		if((gpos.node->shape>>nextchild)&0x1){
			*color=gpos.node->color;
			return true;
		}

		#else
		while(true){
			nextchild=(((gpos.z>>gpos.depth)&0x1)<<2)|(((gpos.y>>gpos.depth)&0x1)<<1)|((gpos.x>>gpos.depth)&0x1);

			b=(!(gpos.node->shape) | !(gpos.node->nodes) | gpos.depth==0);

			if(b | !((gpos.node->shape>>nextchild)&0x1)){
				if(b){
					*color=gpos.node->color;
					return true;
				}
				break;
			}

			gpos.node=&(gpos.node->nodes->next[nextchild]);
			--gpos.depth;
		}
		#endif
		#endif
	}

	return false;
}
#endif
