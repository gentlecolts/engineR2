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

#if 1

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

	if(!(node->nodes) || node->shape==0x00){// || pixrad*tlow*pixrad*tlow>rsqr*denom*denom){
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

		return true;
	}

	vec3d vec;

	//double tlow,thigh;
	long long tlow,thigh;
	double
		x1,x2,
		y1,y2,
		z1,z2;
	#define uselongxor 0

	//if(scale>1){printf("%i\n",scale);}
	/*
	double denom=pow2(-scale);//1.0/(llong(1)<<llong(scale));// 1/(2^scale)
	/*/
	double denom;
	*(long long*)&denom=(0x3ffll-scale-1)<<52;
	//*/

	nodeval valarr[8];
	int numnodes=0;

	bool tmpbool;

	const double
		a=(1-2*(v.x>=0))*w*denom,
		b=(1-2*(v.y>=0))*h*denom,
		c=(1-2*(v.z>=0))*d*denom,
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
		x1=(x1val+varr[i].x*denom)*invdx;
		x2=(x2val+varr[i].x*denom)*invdx;
		y1=(y1val+varr[i].y*denom)*invdy;
		y2=(y2val+varr[i].y*denom)*invdy;
		z1=(z1val+varr[i].z*denom)*invdz;
		z2=(z2val+varr[i].z*denom)*invdz;
	#else
		x1=x1val+varr[i].x*denomx;
		x2=x2val+varr[i].x*denomx;
		y1=y1val+varr[i].y*denomy;
		y2=y2val+varr[i].y*denomy;
		z1=z1val+varr[i].z*denomz;
		z2=z2val+varr[i].z*denomz;
	#endif
		#define lx1 (*(long long*)&x1)
		#define lx2 (*(long long*)&x2)
		#define ly1 (*(long long*)&y1)
		#define ly2 (*(long long*)&y2)
		#define lz1 (*(long long*)&z1)
		#define lz2 (*(long long*)&z2)

		#if uselongxor
		/*
		*(long long*)&tlow=(((lx1>=ly1) & (lx1>=lz1))*lx1)^(((ly1>lx1) & (ly1>=lz1))*ly1)^(((lz1>lx1) & (lz1>ly1))*lz1);
		*(long long*)&thigh=(((lx2<=ly2) & (lx2<=lz2))*lx2)^(((ly2<lx2) & (ly2<=lz2))*ly2)^(((lz2<lx2) & (lz2<ly2))*lz2);
		/*/
		tlow=(((lx1>=ly1) & (lx1>=lz1))*lx1)^(((ly1>lx1) & (ly1>=lz1))*ly1)^(((lz1>lx1) & (lz1>ly1))*lz1);
		thigh=(((lx2<=ly2) & (lx2<=lz2))*lx2)^(((ly2<lx2) & (ly2<=lz2))*ly2)^(((lz2<lx2) & (lz2<ly2))*lz2);
		//*/
		#else
		/*
		*(long long*)&tlow=max(max(lx1,ly1),lz1);
		*(long long*)&thigh=min(min(lx2,ly2),lz2);
		/*/
		tlow=max(max(lx1,ly1),lz1);
		thigh=min(min(lx2,ly2),lz2);
		//*/
		#endif

		/*
		tlow=max(tlow,0.0);
		tmpbool=( ((node->shape>>i)&0x1) & (*(long long*)&tlow<*(long long*)&thigh) );
		*(long long*)&(valarr[i].tlow)^=tmpbool*((*(long long*)&tlow)^(*(long long*)&(valarr[i].tlow)));
		/*/
		tlow=max(tlow,0ll);
		tmpbool=( ((node->shape>>i)&0x1) & (tlow<thigh) );
		valarr[i].tlow^=tmpbool*(tlow^valarr[i].tlow);
		//*/
		valarr[i].index^=tmpbool*(i^valarr[i].index);
		numnodes+=tmpbool;
	}

	if(numnodes==0){
		return false;
	}

	#define longbits(x) (*(long long*)&(x))
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
	#define SWAP(x,y) tmpbool=(valarr[x].tlow<valarr[y].tlow);\
						valarr[x].tlow^=valarr[y].tlow;\
						valarr[x].index^=valarr[y].index;\
						\
						valarr[y].tlow^= !tmpbool * valarr[x].tlow /*if tmpbool is false then this stays the same*/;\
						valarr[y].index^= !tmpbool * valarr[x].index ;\
						\
						valarr[x].tlow^=valarr[y].tlow;\
						valarr[x].index^=valarr[y].index;
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
		vec.x=varr[valarr[i].index].x*denom+p.x;
		vec.y=varr[valarr[i].index].y*denom+p.y;
		vec.z=varr[valarr[i].index].z*denom+p.z;

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



	//note that if thigh<0 implies tlow<0, if tlow<0 then tlow=0 because of the above line, so this will include the check of thigh<0
	/**handle the following cases:
	the ray does not intersect the current node or the cube is completely behind the ray
	ray intersects and this is a leaf node
	*/
	///TODO: add a maxdepth

	//ray intersects the current node, but the node is not a leaf node


}
#else
///TODO: try to make this w/o recursion
bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	//if(scale>1){printf("%i\n",scale);}
	/*
	double denom=pow2(-scale);//1.0/(llong(1)<<llong(scale));// 1/(2^scale)
	/*/
	double denom;
	*(long long*)&denom=(0x3ffll-scale)<<52;
	//*/

	#if infguard0
	double tlow,thigh;
	{
	const double
		a=(1-2*(v.x>=0))*w*denom,
		b=(1-2*(v.y>=0))*h*denom,
		c=(1-2*(v.z>=0))*d*denom,
		px=v0.x-p.x,
		py=(v0.y-p.y),
		pz=(v0.z-p.z),
		x1=(a-px)/v.x,
		x2=(-a-px)/v.x,
		y1=(b-py)/v.y,
		y2=(-b-py)/v.y,
		z1=(c-pz)/v.z,
		z2=(-c-pz)/v.z;
	/*
	*(long long*)&tlow=max(max(*(long long*)&x1,*(long long*)&y1),*(long long*)&z1);
	*(long long*)&thigh=min(min(*(long long*)&x2,*(long long*)&y2),*(long long*)&z2);
	/*/
	const long long
		lx1=*(long long*)&x1,
		lx2=*(long long*)&x2,
		ly1=*(long long*)&y1,
		ly2=*(long long*)&y2,
		lz1=*(long long*)&z1,
		lz2=*(long long*)&z2;
	*(long long*)&tlow=((lx1>=ly1 && lx1>=lz1)*lx1)^((ly1>lx1 && ly1>=lz1)*ly1)^((lz1>lx1 && lz1>ly1)*lz1);
	*(long long*)&thigh=((lx2<=ly2 && lx2<=lz2)*lx2)^((ly2<lx2 && ly2<=lz2)*ly2)^((lz2<lx2 && lz2<ly2)*lz2);
	//*/
	}
	#else
	#define x1 ( (1-2*(v.x>=0)) *w*denom-(v0.x-p.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *w*denom-(v0.x-p.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *h*denom-(v0.y-p.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *h*denom-(v0.y-p.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *d*denom-(v0.z-p.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *d*denom-(v0.z-p.z))/v.z

	double tlow=max(max(x1,y1),z1);
	//const double thigh=min(min(x2,y2),z2);
	double thigh=min(min(x2,y2),z2);

	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2
	#endif

	///TODO: storing normals might be best
	#define norm_light 0
	#if !infguard4
	tlow=max(tlow,0.0);
	#endif

	//note that if thigh<0 implies tlow<0, if tlow<0 then tlow=0 because of the above line, so this will include the check of thigh<0
	/**handle the following cases:
	the ray does not intersect the current node or the cube is completely behind the ray
	ray intersects and this is a leaf node
	*/
	///TODO: add a maxdepth

	#if infguard1
	bool tmpbool=*(long long*)&tlow>*(long long*)&thigh;//inf's are bad D:
	#else
	bool tmpbool=tlow>thigh;
	#endif

	/*
	if(!(tlow<*closeT)){
		return false;
	}
	/*/
	//tmpbool&=(tlow<*closeT);
	//*/

	#if linkparent
	#if infguard2
	if(tmpbool || !(node->nodes) || node->shape==0x00){
	#else
	if(tmpbool || !(node->nodes) || node->shape==0x00 || pixrad*tlow*pixrad*tlow>rsqr*denom*denom){
	#endif
	#else
	#if infguard2
	if(tmpbool || !(node->next) || node->shape==0x00){
	#else
	if(tmpbool || !(node->next) || node->shape==0x00 || pixrad*tlow*pixrad*tlow>rsqr*denom*denom){
	#endif
	#endif
		#if norm_light==0
		(*color)^=(!tmpbool)*((node->color) ^ (*color));
		// *color=node->color;
		#else
		//if(!(tlow>thigh)){
			uint32_t r,g,b;

			const float
				x=abs(tlow*v.x+v0.x-p.x),
				y=abs(tlow*v.y+v0.y-p.y),
				z=abs(tlow*v.z+v0.z-p.z);
			float dot=abs(v.xyz[(y>x && y>z)+2*(z>x && z>y)]);//*(pixrad*tlow*pixrad*tlow<=rsqr*denom*denom);
			//float dot=(abs(v.x)+abs(v.y)+abs(v.z))/3;

			//dot*=(dot>0);
			dot=(dot+1)/2;
			//dot=1-(dot-1)*(dot-1);
			r=(((node->color)>>16)&0xff)*dot;
			g=(((node->color)>>8)&0xff)*dot;
			b=((node->color)&0xff)*dot;
			//*color^=(!tmpbool)*(((r<<16)|(g<<8)|b)^(*color));
			*color=tmpbool?*color:((r<<16)|(g<<8)|b);
		//}
		#endif

		*(long long*)closeT ^= (!tmpbool)*((*(long long*)&tlow)^(*(long long*)closeT));
		//*closeT = tmpbool?*closeT:tlow;
		//*closeT=tlow;

		return !tmpbool;
	}

	//ray intersects the current node, but the node is not a leaf node

	//double tmp;
	#define tmp thigh//instead of making a new double, just reuse thigh
	tmpbool=false;
	uint32_t closeCol,tmpcol;

	tlow=INFINITY;
	++scale;
	denom/=2;

	for(int i=0;i<8;i++){
		#if 1
		if(
			(node->shape>>i)&0x1
			&&
			chkIntersect(
				#if linkparent
				&(node->nodes->next[i]),
				#else
				&(node->next[i]),
				#endif
				vec3d( varr[i].x*denom+p.x , varr[i].y*denom+p.y , varr[i].z*denom+p.z ),
				v,v0,pixrad,&tmpcol,scale,&tmp
			)
		){
			tmpbool=true;

			//closeCol=((tmp<tlow)*tmpcol)^((!(tmp<tlow))*closeCol);
			closeCol^=(tmp<tlow)*(tmpcol^closeCol);

			#if infguard3
			*(long long*)&tlow=min(*(long long*)&tlow,*(long long*)&tmp);
			#else
			tlow=min(tlow,tmp);
			#endif
		}
		#else
		if((node->shape>>i)&0x1)
		tmpbool|=(
			//(node->shape>>i)&0x1
			//&&
			chkIntersect(
				#if linkparent
				&(node->nodes->next[i]),
				#else
				&(node->next[i]),
				#endif
				vec3d( varr[i].x*denom+p.x , varr[i].y*denom+p.y , varr[i].z*denom+p.z ),
				v,v0,pixrad,&closeCol,scale,&tlow
			)
		);

			//closeCol=((tmp<tlow)*tmpcol)^((!(tmp<tlow))*closeCol);
			//closeCol^=(tmp<tlow)*(tmpcol^closeCol);
		#endif
	}

	*color=closeCol;
	*closeT=tlow;
	return tmpbool;
}
#endif
