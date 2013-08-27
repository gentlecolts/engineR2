#include "polygon.h"
#include "vnode.h"
#include "../cam/vec3d.h"
#include <cmath>
#include <algorithm>
using namespace std;

inline bool vertBoxIntersect(const vert& ply,vecref pos,const double& gridsize,const double scale){
}
void vert::placeVert(vnode* node,vec3d v,const long depth,const long maxdepth,double scale){
}

vert::operator vec3d(){
	return vec3d(x,y,z);
}

//see also: http://fileadmin.cs.lth.se/cs/personal/tomas_akenine-moller/pubs/tribox.pdf
//is it faster?
#define AABB 1
#define smooth 1
inline bool triBoxIntersect(const poly& ply,vecref pos,const double scale){
	#if AABB
	uint8_t b=0xff,xyz;

	for(int i=0;i<ply.numvert;i++){
		xyz=(ply.verts[i]->z<pos.z-scale) ^ (2*(ply.verts[i]->z>pos.z+scale));
		xyz<<=2;
		xyz|=(ply.verts[i]->y<pos.y-scale) ^ (2*(ply.verts[i]->y>pos.y+scale));
		xyz<<=2;
		xyz|=(ply.verts[i]->x<pos.x-scale) ^ (2*(ply.verts[i]->x>pos.x+scale));
		b&=xyz;
	}
	#if smooth
	if(b!=0){
		return false;
	}
	#else
	return (b==0);
	#endif

	#if smooth
	vec3d p0,p,p2,v1,v2;
	double t;

	double tlow,thigh;

	for(int i=0;i<ply.numvert;i++){
		p0=*(ply.verts[(i+1)%(ply.numvert)]);
		v1=(vec3d)(*(ply.verts[i]));
		v2=(vec3d)(*(ply.verts[(i+2)%(ply.numvert)]));
		v1-=p0;
		v2-=p0;


		#define tol (0)

		if(
			p0.x>=pos.x-(scale+tol) && p0.x<=pos.x+(scale+tol) &&
			p0.y>=pos.y-(scale+tol) && p0.y<=pos.y+(scale+tol) &&
			p0.z>=pos.z-(scale+tol) && p0.z<=pos.z+(scale+tol)
		){
			return true;
		}

		for(int j=0;j<3;j++){
			for(int k=-1;k<=1;k+=2){
#if 0
				t=(k*scale+pos.xyz[j]-p0.xyz[j])/v1.xyz[j];
				if(t>1 || t<0){
					goto next;
				}
				p=v1;
				p*=t;
				p+=p0;

				xyz=(p.z<pos.z-(scale+tol)) ^ (2*(p.z>pos.z+(scale+tol)));
				xyz<<=2;
				xyz|=(p.y<pos.y-(scale+tol)) ^ (2*(p.y>pos.y+(scale+tol)));
				xyz<<=2;
				xyz|=(p.x<pos.x-(scale+tol)) ^ (2*(p.x>pos.x+(scale+tol)));
				b=xyz;

				t=(k*scale+pos.xyz[j]-p0.xyz[j])/v2.xyz[j];
				if(t>1 || t<0){
					goto next;
				}
				p=v2;
				p*=t;
				p+=p0;

				xyz=(p.z<pos.z-(scale+tol)) ^ (2*(p.z>pos.z+(scale+tol)));
				xyz<<=2;
				xyz|=(p.y<pos.y-(scale+tol)) ^ (2*(p.y>pos.y+(scale+tol)));
				xyz<<=2;
				xyz|=(p.x<pos.x-(scale+tol)) ^ (2*(p.x>pos.x+(scale+tol)));
				b&=xyz;

				if(b==0){
					return true;
				}
#else
				t=(k*scale+pos.xyz[j]-p0.xyz[j])/v1.xyz[j];
				if(t>1 || t<0){
					goto next;
				}
				p=v1;
				p*=t;
				p+=p0;

				t=(k*scale+pos.xyz[j]-p0.xyz[j])/v2.xyz[j];
				if(t>1 || t<0){
					goto next;
				}
				p2=v2;
				p2*=t;
				p2+=p0;
				p2-=p;

	#if 0
				#define tol1 (1e-3)

				#define x1 ( (1-2*(p2.x>=0)) *(scale+tol1)-(p.x-pos.x))/p2.x
				#define x2 ( (2*(p2.x>=0)-1) *(scale+tol1)-(p.x-pos.x))/p2.x
				#define y1 ( (1-2*(p2.y>=0)) *(scale+tol1)-(p.y-pos.y))/p2.y
				#define y2 ( (2*(p2.y>=0)-1) *(scale+tol1)-(p.y-pos.y))/p2.y
				#define z1 ( (1-2*(p2.z>=0)) *(scale+tol1)-(p.z-pos.z))/p2.z
				#define z2 ( (2*(p2.z>=0)-1) *(scale+tol1)-(p.z-pos.z))/p2.z

				tlow=max(max(x1,y1),z1);
				thigh=min(min(x2,y2),z2);

				if(tlow<=thigh && tlow>=0 && tlow<=1){
					return true;
				}
				#undef x1
				#undef x2
				#undef y1
				#undef y2
				#undef z1
				#undef z2
	#else
				#define tol1 (0)
				#define tol2 (1e-8)
				#define a1 ( (1-2*(p2.xyz[(j+1)%3]>=0)) *(scale+tol1)-(p.xyz[(j+1)%3]-pos.xyz[(j+1)%3]))/p2.xyz[(j+1)%3]
				#define a2 ( (2*(p2.xyz[(j+1)%3]>=0)-1) *(scale+tol1)-(p.xyz[(j+1)%3]-pos.xyz[(j+1)%3]))/p2.xyz[(j+1)%3]
				#define b1 ( (1-2*(p2.xyz[(j+2)%3]>=0)) *(scale+tol1)-(p.xyz[(j+2)%3]-pos.xyz[(j+2)%3]))/p2.xyz[(j+2)%3]
				#define b2 ( (2*(p2.xyz[(j+2)%3]>=0)-1) *(scale+tol1)-(p.xyz[(j+2)%3]-pos.xyz[(j+2)%3]))/p2.xyz[(j+2)%3]
				tlow=max(a1,b1);
				thigh=min(a2,b2);

				if(tlow<=thigh && tlow>=0-tol2 && tlow<=1+tol2){
				//if(tlow<=thigh && abs(tlow)<=1+tol){
					return true;
				}
				#undef x1
				#undef x2
				#undef y1
				#undef y2
				#undef z1
				#undef z2
	#endif
#endif
				next:
				;
			}
		}
	}

	return false;

	#endif

	#else
	bool b=false;

	for(int i=0;i<ply.numvert;i++){
		b|=abs( ply.verts[i]->x - pos.x )<=scale &&
			abs(ply.verts[i]->y - pos.y )<=scale &&
			abs( ply.verts[i]->z - pos.z )<=scale;
	}
	if(b){return true;}
//*
	b=false;

	vec3d v0,v;
	double tlow,thigh;

	#define x1 ( (1-2*(v.x>=0)) *scale-(v0.x-pos.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *scale-(v0.x-pos.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *scale-(v0.y-pos.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *scale-(v0.y-pos.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *scale-(v0.z-pos.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *scale-(v0.z-pos.z))/v.z
	for(int i=0;i<3;i++){
		v0=vec3d(
			ply.verts[i]->x,
			ply.verts[i]->y,
			ply.verts[i]->z);
		v=vec3d(
			ply.verts[(i+1)%ply.numverts]->x,
			ply.verts[(i+1)%ply.numverts]->y,
			ply.verts[(i+1)%3]->z)
			-v0;

		tlow=max(max(x1,y1),z1);
		thigh=min(min(x2,y2),z2);

		b|=(tlow<=thigh && tlow>=0 && tlow<=1);
	}
	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2
	if(b){return true;}
	b=false;
//*/

	return false;
	#endif
}

void poly::placePoly(vnode* node,vec3d v,const long depth,const long maxdepth,double scale){
	//printf("depth: %i\n",depth);
	vec3d pos;
	for(int i=0;i<8;i++){
		pos=v+vec3d(scale*(1-2*(i&0x1)),scale*(1-2*((i>>1)&0x1)),scale*(1-2*((i>>2)&0x1)));

		if(triBoxIntersect(*this,pos,scale)){
			if(depth<maxdepth){
				if(!(node->next)){
					node->initChildren(0x1<<i);
				}

				placePoly(&(node->next[i]),pos,depth+1,maxdepth,scale/2);
			}//*
			else{
				uint32_t a=0,r=0,g=0,b=0,c=0;
				for(int j=0;j<numvert;j++){
					c+=verts[j]->hascolor;
					a+=(verts[j]->hascolor) * ((verts[j]->color>>24)&0xff);
					r+=(verts[j]->hascolor) * ((verts[j]->color>>16)&0xff);
					g+=(verts[j]->hascolor) * ((verts[j]->color>>8)&0xff);
					b+=(verts[j]->hascolor) * (verts[j]->color&0xff);
				}
				const uint32_t c2=(c<1) ^ ((!(c<1))*c);
				a/=c2;
				r/=c2;
				g/=c2;
				b/=c2;

				a=(a+(hascolor)*((color>>24)&0xff))>>(hascolor);//divide by two if hasColor
				r=(r+(hascolor)*((color>>16)&0xff))>>(hascolor);//divide by two if hasColor
				g=(g+(hascolor)*((color>>8)&0xff))>>(hascolor);//divide by two if hasColor
				b=(b+(hascolor)*(color&0xff))>>(hascolor);//divide by two if hasColor

				//printf("c: %i\thascolor: %b\tcolor: %x %x %x %x\n",c,hascolor,a,r,g,b);

				node->color^=(c>0 || hascolor)*( ((a<<24)|(r<<16)|(g<<8)|b) ^ node->color);
			}//*/

			node->shape|=0x1<<i;
		}
	}
}
