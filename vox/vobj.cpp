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

	invmtr[0]=yvec.y*zvec.z-zvec.y*yvec.z;invmtr[1]=zvec.x*yvec.z-yvec.x*zvec.z;invmtr[2]=yvec.x*zvec.y-yvec.y*zvec.x;
	invmtr[3]=zvec.y*xvec.z-xvec.y*zvec.z;invmtr[4]=xvec.x*zvec.z-zvec.x*xvec.z;invmtr[5]=zvec.x*xvec.y-xvec.x*zvec.y;
	invmtr[6]=xvec.y*yvec.z-yvec.y*xvec.z;invmtr[7]=yvec.x*xvec.z-xvec.x*yvec.z;invmtr[8]=xvec.x*yvec.y-yvec.x*xvec.y;
	invdet=1/(xvec.x*(yvec.y*zvec.z-zvec.y*yvec.z)-yvec.x*(xvec.y*zvec.z-zvec.y*xvec.z)+zvec.x*(xvec.y*yvec.z-yvec.y*xvec.z));
}

///TODO: fix this stupid shit...there's no need for all this function call overhead
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
