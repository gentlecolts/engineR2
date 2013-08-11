#ifndef POLYGON_H_INCLUDED
#define POLYGON_H_INCLUDED
#include <stdint.h>
struct vnode;
struct vec3d;

struct vert{
	union{
		struct{double x,y,z;};
		double xyz[3];
	};
	uint32_t color;
	bool hascolor;

	vert(double x0=0,double y0=0,double z0=0):x(x0),y(y0),z(z0),color(0xff000000),hascolor(false){}

	operator vec3d();

	void placeVert(vnode* node,vec3d v,const long depth,const long maxdepth,double scale);
};
struct poly{
	vert** verts;
	int numvert;
	uint32_t color;
	bool hascolor;

	poly():verts(__null),numvert(0),color(0xff000000),hascolor(false){}
	~poly(){
		delete[] verts;
	}

	void placePoly(vnode* node,vec3d v,const long depth,const long maxdepth,double scale);
};

#endif // POLYGON_H_INCLUDED
