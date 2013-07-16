#ifndef VOBJ_H_INCLUDED
#define VOBJ_H_INCLUDED

#include "vnode.h"

class vobj;
typedef const vobj& vobjref;

class vobj{
protected:
	virtual bool chkIntersect(vnode* node,vec3d p,vecref v,double vx0,double vy0,double vz0,uint32_t* color,int scale,double* closeT) const;
	vec3d varr[6];
public:
	vec3d pos,xvec,yvec,zvec;
	vnode* head;
	double w,h,d;

	vobj(double wid=0,double hght=0,double dep=0,double x0=0,double y0=0,double z0=0);
	vobj(vobjref o);
	virtual ~vobj();

	virtual vobjref operator =(vobjref o);

	virtual void updateVals();//TODO: consider making xvec,yvec,zvec private
	virtual bool intersects(vecref vec,vecref origin,uint32_t* color,double* closeT=NULL) const;
	virtual bool intersects(vecref vec,double x0,double y0,double z0,uint32_t* color,double* closeT=NULL) const;
};
#endif // VOBJ_H_INCLUDED
