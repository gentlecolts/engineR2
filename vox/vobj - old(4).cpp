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

bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const{
	vec3d
		p0(
			(v0.x-p.x),
			(v0.y-p.y),
			(v0.z-p.z)
		),
		delta(v.x,v.y,v.z);

	const double
		x1=( (1-2*(v.x>=0))*w -p0.x)/delta.x,
		x2=( (2*(v.x>=0)-1)*w -p0.x)/delta.x,
		y1=( (1-2*(v.y>=0))*h -p0.y)/delta.y,
		y2=( (2*(v.y>=0)-1)*h -p0.y)/delta.y,
		z1=( (1-2*(v.z>=0))*d -p0.z)/delta.z,
		z2=( (2*(v.z>=0)-1)*d -p0.z)/delta.z;

	#define longbits(x) (*(long long*)&(x))

	double tlow,thigh;
	longbits(tlow)=max(max(max(longbits(x1),longbits(y1)),longbits(z1)),0ll);
	longbits(thigh)=min(min(longbits(x2),longbits(y2)),longbits(z2));
	if(longbits(tlow)>longbits(thigh)){
		return false;
	}

	typedef uint32_t dtype;//depth type
	const dtype gsize=(0x1ll<<maxdepth);

	#define dconst 1

	#define tsimp 1

	#if tsimp
	/*
	x0=(((x>>depth)+(delta.x>0))<<depth)
	(x0-gsize*(p0.x+w+tlow*delta.x)/(2*w))/(delta.x/(2*w));
	(x0-gsize*(p0.x+w+tlow*delta.x)/(2*w))*2*w/delta.x;
	(x0*2*w-gsize*(p0.x+w+tlow*delta.x)/(2*w)*2*w)/delta.x;
	(x0*2*w-gsize*(p0.x+w+tlow*delta.x))/delta.x;
	x0*2*w/delta.x-gsize*(p0.x+w+tlow*delta.x)/delta.x;
	*/
	const double
		dxinv=dconst*w/delta.x,
		dyinv=dconst*h/delta.y,
		dzinv=dconst*d/delta.z;
	#endif

	/*
	p0.x=((p0.x+tlow*delta.x)/w+1)/2;
	p0.y=((p0.y+tlow*delta.y)/h+1)/2;
	p0.z=((p0.z+tlow*delta.z)/d+1)/2;
	/*/
	p0.x=(p0.x+w+tlow*delta.x)/(2*w);
	p0.y=(p0.y+h+tlow*delta.y)/(2*h);
	p0.z=(p0.z+d+tlow*delta.z)/(2*d);
	//*/

	delta.x/=dconst*w;
	delta.y/=dconst*h;
	delta.z/=dconst*d;

	/*
	p0.x=(p0.x>=0)?(p0.x<1)?p0.x:(gsize-1.)/gsize:0;
	p0.y=(p0.y>=0)?(p0.y<1)?p0.y:(gsize-1.)/gsize:0;
	p0.z=(p0.z>=0)?(p0.z<1)?p0.z:(gsize-1.)/gsize:0;
	//*/

	/*
	dtype
		x=p0.x*gsize,
		y=p0.y*gsize,
		z=p0.z*gsize,
	/*/
	dtype
		x=(p0.x>=0)?(p0.x<1)?p0.x*gsize:gsize-1:0,
		y=(p0.y>=0)?(p0.y<1)?p0.y*gsize:gsize-1:0,
		z=(p0.z>=0)?(p0.z<1)?p0.z*gsize:gsize-1:0,
	//*/
		depth;
	uint8_t nextchild;
	bool b;

	vnode* tmpnode;

	#define oneconst 1

	long long txy,txz,tyz;

	#define skipblock 1

	//delta.normalize();

	const double
		invdx=1/(delta.x*gsize),
		invdy=1/(delta.y*gsize),
		invdz=1/(delta.z*gsize);
	#if skipblock
	p0*=gsize;
	#else
	p0.x=(delta.x>0)-p0.x*gsize;
	p0.y=(delta.y>0)-p0.y*gsize;
	p0.z=(delta.z>0)-p0.z*gsize;
	#endif

	char tmpstr[33];

	const int
		sgndx=(delta.x>0)-(delta.x<0),
		sgndy=(delta.y>0)-(delta.y<0),
		sgndz=(delta.z>0)-(delta.z<0);

	int i=0;

	#define go_up 0
	#define oneloop 1
	#define storemin 0

	#if go_up
	tmpnode=node;
	depth=maxdepth-1;
	dtype lastx=0,lasty=0,lastz=0;

	#if oneloop
	bool b2;
	#endif
	#endif

	#if storemin
	long long tmin;
	#endif

	//while((x<gsize) & (x>=0) & (y<gsize) & (y>=0) & (z<gsize) & (z>=0)){
	//while((x<gsize) & (x>=0) & (y<gsize) & (y>=0) & (z<gsize) & (z>=0) & (i<7*gsize+8)){
	while((x<gsize) & (x>=0) & (y<gsize) & (y>=0) & (z<gsize) & (z>=0) & (i<(gsize<<2))){
	//while((x<gsize) & (y<gsize) & (z<gsize) & (i<7*gsize+8)){
	//while((x<gsize) & (y<gsize) & (z<gsize)){

		#if go_up

		#if oneloop
		b2=false;
		#else
		++depth;

		//bool passroot=false;
		while(
			((x>>depth)!=lastx) |
			((y>>depth)!=lasty) |
			((z>>depth)!=lastz)
		){
			//printf("last: ( %ux , %ux , %ux )\n",lastx,lasty,lastz);
			lastx>>=1;
			lasty>>=1;
			lastz>>=1;
			++depth;
			//passroot|=(tmpnode==node);
			tmpnode=tmpnode->getParent();
		}

		//if(passroot){printf("went past root: ( %ux , %ux , %ux )\tlast: ( %ux , %ux , %ux )\n",x,y,z,lastx,lasty,lastz);}

		--depth;
		#endif

		while(true){
			nextchild=(((z>>depth)&0x1)<<2)|(((y>>depth)&0x1)<<1)|((x>>depth)&0x1);
			nextchild^=0x7;

			//b=(!(tmpnode->shape) | !(tmpnode->nodes));
			//b=(!(tmpnode->shape) | !(tmpnode->nodes) | depth==0);
			b=(!(tmpnode->shape) | !(tmpnode->nodes) | (depth==-1));

			#if oneloop
			b2|=((x>>(depth+1))==lastx) &
				((y>>(depth+1))==lasty) &
				((z>>(depth+1))==lastz);
			#endif

			if(b2 & (b | !((tmpnode->shape>>nextchild)&0x1))){
				if(b){
					*color=tmpnode->color;
					return true;
				}
				break;
			}

			#if oneloop
			//*
			tmpnode=tmpnode->getParent();
			// (*(long long*)&tmpnode)^=b2*((long long)(long long*)&(tmpnode->nodes->next[nextchild])^(long long)(long long*)tmpnode);
			 (*(long long*)&tmpnode)^=b2*((long long)&(tmpnode->nodes->next[nextchild])^(long long)tmpnode);
			//((long long&)tmpnode)^=b2*((long long)&(tmpnode->nodes->next[nextchild])^(long long)tmpnode);
			/*/
			tmpnode=b2?&(tmpnode->nodes->next[nextchild]):tmpnode->getParent();
			//*/
			depth+=b2?-1:1;
			#else
			tmpnode=&(tmpnode->nodes->next[nextchild]);
			--depth;
			#endif
		}
		/*
		lastx=x>>(depth+1);
		lasty=y>>(depth+1);
		lastz=z>>(depth+1);
		//*/
		#else
		tmpnode=node;
		depth=maxdepth-1;

		while(true){
			nextchild=(((z>>depth)&0x1)<<2)|(((y>>depth)&0x1)<<1)|((x>>depth)&0x1);
			//nextchild=((z>>(depth-2))&0x4)|((y>>(depth-1))&0x2)|((x>>depth)&0x1);
			nextchild^=0x7;

			b=(!(tmpnode->shape) | !(tmpnode->nodes));
			//b=(!(tmpnode->shape) | !(tmpnode->nodes) | depth==0);
			//b=(!(tmpnode->shape) | !(tmpnode->nodes) | (depth==-1));

			if(b | !((tmpnode->shape>>nextchild)&0x1)){
				if(b){
					*color=tmpnode->color;
					return true;
				}
				//printf("depth: %i\n",depth);
				break;//depth is 0 at break
			}

			tmpnode=&(tmpnode->nodes->next[nextchild]);
			--depth;
		}
		#endif

		/*
		t=(x+(dx>0)-x0*gsize)/(dx*gsize)
		t=((x+(dx>0))/gsize-x0)/dx
		t*dx+x0=(x+(dx>0))/gsize
		*/

		#if skipblock
		*(double*)&tyz=((((x>>depth)+(delta.x>0))<<depth)-p0.x)*dxinv;
		*(double*)&txz=((((y>>depth)+(delta.y>0))<<depth)-p0.y)*dyinv;
		*(double*)&txy=((((z>>depth)+(delta.z>0))<<depth)-p0.z)*dzinv;
		#else
		*(double*)&tyz=(x+p0.x)*invdx;
		*(double*)&txz=(y+p0.y)*invdy;
		*(double*)&txy=(z+p0.z)*invdz;
		#endif

		#if storemin
		tmin=min(min(txy,txz),tyz);
		x+=(tyz==tmin)*sgndx;
		y+=(txz==tmin)*sgndy;
		z+=(txy==tmin)*sgndz;
		#else

		#if skipblock
		if((tyz<=txy) & (tyz<=txz)){
			//const int dx=(((x+sgndx*(0x1<<depth))>>depth)<<depth)-x;
			//const int dx=(((x>>depth)+sgndx)<<depth)-x;
			//const int dx=((delta.x>0)-p0.x+gsize*tyz*delta.x)-x;
			//const int dx=(p0.x+tyz*delta.x)-x;
			//const int dx=(((x+(delta.x>0)*(0x1<<depth))>>depth)<<depth)-x;
			const int dx=(((x>>depth)+(delta.x>0))<<depth)-x;
			x+=dx-(delta.x<0);
			y+=(delta.y*dx)/delta.x;
			z+=(delta.z*dx)/delta.x;
		}else if((txz<=txy) & (txz<=tyz)){
			//const int dy=(((y+sgndy*(0x1<<depth))>>depth)<<depth)-y;
			//const int dy=(((y>>depth)+sgndy)<<depth)-y;
			//const int dy=((delta.y>0)-p0.y+gsize*txz*delta.y)-y;
			//const int dy=(p0.y+txz*delta.y)-y;
			//const int dy=(((y+(delta.y>0)*(0x1<<depth))>>depth)<<depth)-y;
			const int dy=(((y>>depth)+(delta.y>0))<<depth)-y;
			y+=dy-(delta.y<0);
			x+=(delta.x*dy)/delta.y;
			z+=(delta.z*dy)/delta.y;
		}else{
			//const int dz=(((z+sgndz*(0x1<<depth))>>depth)<<depth)-z;
			//const int dz=(((y>>depth)+sgndy)<<depth)-y;
			//const int dz=((delta.z>0)-p0.z+gsize*txy*delta.z)-z;
			//const int dz=(p0.z+txy*delta.z)-z;
			//const int dz=(((z+(delta.z>0)*(0x1<<depth))>>depth)<<depth)-z;
			const int dz=(((z>>depth)+(delta.z>0))<<depth)-z;
			z+=dz-(delta.z<0);
			y+=(delta.y*dz)/delta.z;
			x+=(delta.x*dz)/delta.z;
		}
		#else
		x+=((tyz<=txy) & (tyz<=txz))*sgndx;
		y+=((txz<=txy) & (txz<=tyz))*sgndy;
		z+=((txy<=txz) & (txy<=tyz))*sgndz;
		#endif
		#endif

		++i;
	}

	return false;
}
#endif
