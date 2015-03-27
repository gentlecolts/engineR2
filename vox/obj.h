#ifndef OBJ_H_INCLUDED
#define OBJ_H_INCLUDED

#include <stdint.h>

class obj{
public:
	vec3d pos,xvec,yvec,zvec;

	virtual void updateVals()=0;//function keeping internal values up to date

	///TODO:consider making these const
	//intersection tests
	virtual bool intersects(vecref vec,vecref origin,const double pixrad,uint32_t* color,double* closeT=NULL)=0;
	virtual bool intersects(vecref vec,double x0,double y0,double z0,const double pixrad,uint32_t* color,double* closeT=NULL)=0;
};

#endif // OBJ_H_INCLUDED
