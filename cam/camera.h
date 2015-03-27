#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "matrix.h"
#include "vec3d.h"
#include "../msc/surface.h"
#include "../vox/obj.h"

matrix getRotMatrix(vec3d u,double theta);

struct point2d{
	double x,y;
	point2d(double x0=0,double y0=0):x(x0),y(y0){}
};

class camera{
protected:
	matrix R;
	surface* scrn;
	uint32_t* pixels;
	double focusLen;
	double aspctRatio,width;
	double viewangle;
	vec3d dir,xvec,yvec;///TODO: it might be benificial to store a normalized dir
	double tilt;
	double rad,inc,azm;//of
	double* depthbuf;
public:
	vec3d pos;//center of the SCREEN, not the origin

	uint32_t bgCol;

	camera(surface* target,double viewangle,double x0=0,double y0=0,double z0=0);//,double dirx=0,double diry=0,double dirz=1);

	virtual point2d getCoords(double x,double y,double z,bool* visible=NULL);
	virtual point2d getCoords(vecref v,bool* visible=NULL);
	virtual void drawPoint(double x,double y,double z,uint32_t color);
	virtual void drawPoint(vecref p,uint32_t color);
	virtual void drawLine(double x0,double y0,double z0,double x1,double y1,double z1,uint32_t color);
	virtual void drawLine(vecref p0,vecref p1,uint32_t color);

	virtual void traceScene(obj* object);//for now it only traces one object at a time, this would likely be a linked list
	virtual void traceScene(int numobj,obj* object);//for now it only traces one object at a time, this would likely be a linked list

	virtual void setViewangle(double angle);
	virtual double getViewangle() const;

	virtual void setRatio(double YoverX);
	virtual void setCamWidth(double wid);

	virtual void lookAt(double x,double y,double z);
	virtual void lookInDir(vec3d& v);

	virtual void setIncline(double theta);
	virtual void setAzimuth(double theta);
	virtual void setTilt(double theta);

	virtual void addIncline(double theta);
	virtual void addAzimuth(double theta);
	virtual void addTilt(double theta);

	virtual void setAngles(double incl,double azim);
	virtual void setAngles(double incl,double azim,double tlt);
	virtual void addToAngles(double incl,double azim);
	virtual void addToAngles(double incl,double azim,double tlt);

	virtual double getIncline() const;
	virtual double getAzimuth() const;
	virtual double getTilt() const;

	virtual void translate(double x,double y,double z);
	virtual void translate(vecref v);
	virtual void translateOriented(double x,double y,double z);
	virtual void translateOriented(vecref v);

	virtual void moveTo(double x,double y,double z);
	virtual void moveTo(vecref v);

	virtual vec3d getXvec();
	virtual vec3d getYvec();
	virtual vec3d getZvec();

	virtual void orbitd(double angle,const vec3d& axis);
	virtual void orbitd(double angle,const vec3d& axis,const vec3d& origin);
	virtual void orbitr(double angle,const vec3d& axis);
	virtual void orbitr(double angle,const vec3d& axis,const vec3d& origin);

	virtual void rotated(double angle,const vec3d& axis);
	virtual void rotatef(double angle,const vec3d& axis);
};

#endif // CAMERA_H_INCLUDED
