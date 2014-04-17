#ifndef VOBJ_H_INCLUDED
#define VOBJ_H_INCLUDED

#include "vnode.h"
#include <string>
#include <fstream>
#include <queue>
class vobj;
typedef const vobj& vobjref;

class vobj{
protected:
	virtual bool chkIntersect(vnode* node,vec3d p,vecref v,vecref v0,const double& pixrad,uint32_t* color,int scale,double* closeT) const;
	vec3d varr[8];
	double rsqr;

	virtual void writeout(std::queue<char>* q,vnode* node);

	double invmtr[9],invdet;
public:
	vec3d pos,xvec,yvec,zvec;
	vnode* head;

	double w,h,d;

	int maxdepth;

	vobj(double wid=0,double hght=0,double dep=0,double x0=0,double y0=0,double z0=0);
	vobj(vobjref o);
	virtual ~vobj();

	virtual vobjref operator =(vobjref o);

	virtual void updateVals();//TODO: consider making xvec,yvec,zvec private
	virtual bool intersects(vecref vec,vecref origin,const double pixrad,uint32_t* color,double* closeT=NULL) const;
	virtual bool intersects(vecref vec,double x0,double y0,double z0,const double pixrad,uint32_t* color,double* closeT=NULL) const;

	virtual void writeToFile(std::string filename);
	virtual void readFromFile(std::string filename);
};

#endif // VOBJ_H_INCLUDED
