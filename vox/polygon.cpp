#include "polygon.h"
#include "vnode.h"
#include "../cam/vec3d.h"
#include <cmath>
#include <algorithm>
#include <omp.h>
using namespace std;

inline bool vertBoxIntersect(const vert& ply,vecref pos,const double& gridsize,const double scale){
}
void vert::placeVert(vnode* node,vec3d v,const long depth,const long maxdepth,double scale){
}

vert::operator vec3d(){
	return vec3d(x,y,z);
}

#define type 2

#if type==0
//original code from http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine_Moller/code
//via http://realtimerendering.com/intersections.html

#define FINDMINMAX(x0,x1,x2,min,max) \
	min=max=x0;\
	if(x1<minval){minval=x1;}\
	if(x1>maxval){maxval=x1;}\
	if(x2<minval){minval=x2;}\
	if(x2>maxval){maxval=x2;}

bool planeBoxOverlap(vecref normal,vecref vert,vec3d maxbox){
	int q;
	vec3d vmin,vmax;
	double v;

	for(q=0;q<3;q++){
		v=vert.xyz[q];
		if(normal.xyz[q]>0){
			vmin.xyz[q]=-maxbox.xyz[q]-v;
			vmax.xyz[q]=maxbox.xyz[q]-v;
		}else{
			vmin.xyz[q]=maxbox.xyz[q]-v;
			vmax.xyz[q]=-maxbox.xyz[q]-v;
		}
	}

	if(normal.dot(vmin)>0){return false;}
	if(normal.dot(vmax)>=0){return true;}

	return false;
}

//=== x-tests ===
#define AXISTEST_X01(a,b,fa,fb) \
	p0=a*v0.y - b*v0.z;\
	p2=a*v2.y - b*v2.z;\
	if(p0<p2){minval=p0;maxval=p2;} else{minval=p2;maxval=p0;}\
	rad=fa*boxhalfsize.y+fb*boxhalfsize.z;\
	if(minval>rad || maxval<-rad){return false;}

#define AXISTEST_X2(a,b,fa,fb) \
	p0=a*v0.y - b*v0.z;\
	p1=a*v1.y - b*v1.z;\
	if(p0<p1){minval=p0;maxval=p1;} else{minval=p1;maxval=p0;}\
	rad=fa*boxhalfsize.y+fb*boxhalfsize.z;\
	if(minval>rad || maxval<-rad){return false;}

//=== y-tests ===
#define AXISTEST_Y02(a,b,fa,fb) \
	p0=-a*v0.x + b*v0.z;\
	p2=-a*v2.x + b*v2.z;\
	if(p0<p2){minval=p0;maxval=p2;} else{minval=p2;maxval=p0;}\
	rad=fa*boxhalfsize.x+fb*boxhalfsize.z;\
	if(minval>rad || maxval<-rad){return false;}

#define AXISTEST_Y1(a,b,fa,fb) \
	p0=-a*v0.x + b*v0.z;\
	p1=-a*v1.x + b*v1.z;\
	if(p0<p1){minval=p0;maxval=p1;} else{minval=p1;maxval=p0;}\
	rad=fa*boxhalfsize.x+fb*boxhalfsize.z;\
	if(minval>rad || maxval<-rad){return false;}

//=== z-tests ===
#define AXISTEST_Z12(a,b,fa,fb) \
	p1=a*v1.x - b*v1.y;\
	p2=a*v2.x - b*v2.y;\
	if(p2<p1){minval=p2;maxval=p1;} else{minval=p1;maxval=p2;}\
	rad=fa*boxhalfsize.x+fb*boxhalfsize.y;\
	if(minval>rad || maxval<-rad){return false;}

#define AXISTEST_Z0(a,b,fa,fb) \
	p0=a*v0.x - b*v0.y;\
	p1=a*v1.x - b*v1.y;\
	if(p0<p1){minval=p0;maxval=p1;} else{minval=p1;maxval=p0;}\
	rad=fa*boxhalfsize.x+fb*boxhalfsize.y;\
	if(minval>rad || maxval<-rad){return false;}

inline bool triBoxIntersect(const poly& ply,vecref boxcenter,const double scale){
	/**
	use separating axis theorem to test overlap between triangle and box
	need to test for overlap in these directions:
	1) the {x,y,z} directions (actually, since we use the AABB of the triangle,
		we do not even need to test these)
	2) normal of the triangle
	3) cross product(edge from tri, {x,y,z}-direction)
		this gives 3x3=9 more tests
	*/

	const vec3d boxhalfsize(scale,scale,scale);

	vec3d
		v0,v1,v2,
		axis;
	double minval,maxval,p0,p1,p2,rad,fex,fey,fez;
	vec3d normal,e0,e1,e2;

	//boxcenter is (0,0,0)
	v0=(vec3d)(*(ply.verts[0]))-boxcenter;
	v1=(vec3d)(*(ply.verts[1]))-boxcenter;
	v2=(vec3d)(*(ply.verts[2]))-boxcenter;

	//compute edges
	e0=v1-v0;
	e1=v2-v1;
	e2=v0-v2;

	/* STEP 3:
	apparently doing the 9 tests first is faster
	*/
	fex=abs(e0.x);
	fey=abs(e0.y);
	fez=abs(e0.z);
	AXISTEST_X01(e0.z,e0.y,fez,fey);
	AXISTEST_Y02(e0.z,e0.x,fez,fex);
	AXISTEST_Z12(e0.y,e0.x,fey,fex);

	fex=abs(e1.x);
	fey=abs(e1.y);
	fez=abs(e1.z);
	AXISTEST_X01(e1.z,e1.y,fez,fey);
	AXISTEST_Y02(e1.z,e1.x,fez,fex);
	AXISTEST_Z12(e1.y,e1.x,fey,fex);

	fex=abs(e2.x);
	fey=abs(e2.y);
	fez=abs(e2.z);
	AXISTEST_X01(e2.z,e2.y,fez,fey);
	AXISTEST_Y02(e2.z,e2.x,fez,fex);
	AXISTEST_Z12(e2.y,e2.x,fey,fex);

	/* STEP 1:
	first test overlap in the {x,y,z}-directions
	find min,max of the triangles each direction, and test for overlap in
	that direction -- this is equivilant to testing a minimal AABB around
	the triangle against the AABB
	*/

	//test in the x-direction
	FINDMINMAX(v0.x,v1.x,v2.x,minval,maxval);
	if(minval>boxhalfsize.x || maxval<-boxhalfsize.x){return false;}

	//test in the y-direction
	FINDMINMAX(v0.y,v1.y,v2.y,minval,maxval);
	if(minval>boxhalfsize.y || maxval<-boxhalfsize.y){return false;}

	//test in the z-direction
	FINDMINMAX(v0.z,v1.z,v2.z,minval,maxval);
	if(minval>boxhalfsize.z || maxval<-boxhalfsize.z){return false;}

	/* STEP 2:
	test if the box intersects the plane of the triangle
	compute plane equation of triangle: normal*x+d=0
	*/

	normal=e0.cross(e1);

	if(!planeBoxOverlap(normal,v0,boxhalfsize)){return false;}

	return true;
}

#elif type==1
//my own implementation of the separating axis theorem
inline bool triBoxIntersect(const poly& ply,vecref pos,const double scale){
	double minval,maxval;
	for(int i=0;i<3;i++){
		minval=INFINITY;
		maxval=-INFINITY;
		for(int j=0;j<ply.numvert;j++){
			minval=min(ply.verts[j]->xyz[i]-pos.xyz[i],minval);
			maxval=max(ply.verts[j]->xyz[i]-pos.xyz[i],maxval);
		}

		if((maxval>scale && minval>scale) || (maxval<scale && minval<scale)){return false;}
	}

	#if 1

	#else
	double minval2,maxval2,dot;
	vec3d normal,
		v0,
		v1(ply.verts[0]->x-pos.x,ply.verts[0]->y-pos.y,ply.verts[0]->z-pos.z),
		v2(ply.verts[1]->x-pos.x,ply.verts[1]->y-pos.y,ply.verts[1]->z-pos.z);
	for(int i=0;i<ply.numvert;i++){
		minval=INFINITY;
		maxval=-INFINITY;
		minval2=INFINITY;
		maxval2=-INFINITY;

		v0=v1;
		v1=v2;
		v2.x=ply.verts[(i+2)%ply.numvert]->x-pos.x;
		v2.y=ply.verts[(i+2)%ply.numvert]->y-pos.y;
		v2.z=ply.verts[(i+2)%ply.numvert]->z-pos.z;

		normal=(v1-v0).cross(v2-v0);
		normal=(v2-v1).cross(normal);

		for(int j=0;j<ply.numvert;j++){
			dot=normal.dot(*(ply.verts[j]));
			minval=min(dot,minval);
			maxval=max(dot,maxval);
		}

		for(int j=0;j<8;j++){
			dot=normal.x*(1-((j<<1)&0x2))+normal.y*(1-(j&0x2))+normal.z*(1-((j>>1)&0x2));
			minval2=min(dot,minval2);
			maxval2=max(dot,maxval2);
		}

		if((maxval>maxval2 && minval>maxval2) || (maxval<minval2 && minval<minval2)){return false;}
	}
	#endif

	return true;
}
#else
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
	double t1,t2;

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
			for(int k=-1;k<=1;k+=2){//using this second loop produces faster code, so it will be kept for now
				/*
				t1=(k*scale+pos.xyz[j]-p0.xyz[j])/v1.xyz[j];
				t2=(k*scale+pos.xyz[j]-p0.xyz[j])/v2.xyz[j];
				/*/
				t1=(k*scale+pos.xyz[j]-p0.xyz[j]);
				t2=t1/v2.xyz[j];
				t1/=v1.xyz[j];
				//*/

				if(!(t1>1 || t1<0 || t2>1 || t2<0)){
					//*
					p=v1;
					p*=t1;
					p+=p0;

					p2=v2;
					p2*=t2;
					p2+=p0;
					p2-=p;
					/*/
					p2=v2;p=v1;

					p*=t1;p2*=t2;

					p2+=p0;p+=p0;
					p2-=p;
					//*/

					#define tol1 (0)
					#define tol2 (1e-16)
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
				}
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
#endif

#if 1
void poly::placePoly(vnode* node,vec3d v,const long depth,const long maxdepth,double scale) const{
	if(depth==maxdepth){
		uint32_t a=0,r=0,g=0,b=0,c=0;
		for(int j=0;j<numvert;j++){
			c+=verts[j]->hascolor;
			a+=(verts[j]->hascolor) * ((verts[j]->color>>24)&0xff);
			r+=(verts[j]->hascolor) * ((verts[j]->color>>16)&0xff);
			g+=(verts[j]->hascolor) * ((verts[j]->color>>8)&0xff);
			b+=(verts[j]->hascolor) * (verts[j]->color&0xff);
		}
		c+=c<1;//avoid division by zero
		a/=c;
		r/=c;
		g/=c;
		b/=c;

		a=(a+(hascolor)*((color>>24)&0xff))>>(hascolor);//divide by two if hasColor
		r=(r+(hascolor)*((color>>16)&0xff))>>(hascolor);//divide by two if hasColor
		g=(g+(hascolor)*((color>>8)&0xff))>>(hascolor);//divide by two if hasColor
		b=(b+(hascolor)*(color&0xff))>>(hascolor);//divide by two if hasColor

		node->color^=(c>0 || hascolor)*( ((a<<24)|(r<<16)|(g<<8)|b) ^ node->color);
		return;
	}

	vec3d pos;

	#if linkparent
	if(!(node->nodes)){
	#else
	if(!(node->next)){
	#endif
	node->initChildren(0);
	}

	for(int i=0;i<8;i++){
		pos=v;
		/*
		pos+=vec3d(scale*(1-((i<<1)&0x2)),scale*(1-(i&0x2)),scale*(1-((i>>1)&0x2)));
		/*/
		pos.x+=scale*(1-((i<<1)&0x2));
		pos.y+=scale*(1-(i&0x2));
		pos.z+=scale*(1-((i>>1)&0x2));
		//*/

		if(triBoxIntersect(*this,pos,scale)){
			#if linkparent
			placePoly(&(node->nodes->next[i]),pos,depth+1,maxdepth,scale/2);
			#else
			placePoly(&(node->next[i]),pos,depth+1,maxdepth,scale/2);
			#endif
			node->shape|=0x1<<i;
		}
	}

	//*
	if(!(node->shape)){
		node->die();
	}//*/
}
#else
void poly::polyplacefunc(vnode* node,vec3d v,const long depth,const long maxdepth,double scale) const{
	if(depth==maxdepth){
		uint32_t a=0,r=0,g=0,b=0,c=0;
		for(int j=0;j<numvert;j++){
			c+=verts[j]->hascolor;
			a+=(verts[j]->hascolor) * ((verts[j]->color>>24)&0xff);
			r+=(verts[j]->hascolor) * ((verts[j]->color>>16)&0xff);
			g+=(verts[j]->hascolor) * ((verts[j]->color>>8)&0xff);
			b+=(verts[j]->hascolor) * (verts[j]->color&0xff);
		}
		c+=c<1;//avoid division by zero
		a/=c;
		r/=c;
		g/=c;
		b/=c;

		a=(a+(hascolor)*((color>>24)&0xff))>>(hascolor);//divide by two if hasColor
		r=(r+(hascolor)*((color>>16)&0xff))>>(hascolor);//divide by two if hasColor
		g=(g+(hascolor)*((color>>8)&0xff))>>(hascolor);//divide by two if hasColor
		b=(b+(hascolor)*(color&0xff))>>(hascolor);//divide by two if hasColor

		node->color^=(c>0 || hascolor)*( ((a<<24)|(r<<16)|(g<<8)|b) ^ node->color);
		return;
	}

	vec3d pos;

	for(int i=0;i<8;i++){
		pos=v;
		pos+=vec3d(scale*(1-((i<<1)&0x2)),scale*(1-(i&0x2)),scale*(1-((i>>1)&0x2)));

		if(triBoxIntersect(*this,pos,scale)){
			if(!(node->next)){
				node->initChildren(0x1<<i);
			}

			node->shape|=0x1<<i;
			placePoly(&(node->next[i]),pos,depth+1,maxdepth,scale/2);
		}
	}
}

void poly::placePoly(vnode* node,vec3d v,const long depth,const long maxdepth,double scale) const{
	if(depth>maxdepth || depth<1){
		return;
	}

	if(!(node->next)){
		node->initChildren(0);
	}

	vec3d pos;
	uint32_t shape=0;
	//#pragma omp parallel for reduction( | : shape)
	for(int i=0;i<8;i++){
		pos=v;
		pos+=vec3d(scale*(1-((i<<1)&0x2)),scale*(1-(i&0x2)),scale*(1-((i>>1)&0x2)));

		if(triBoxIntersect(*this,pos,scale)){
			shape|=0x1<<i;
			polyplacefunc(&(node->next[i]),pos,depth+1,maxdepth,scale/2);
		}
	}

	node->shape=shape;
}
#endif
