#ifndef VEC3D_H_INCLUDED
#define VEC3D_H_INCLUDED

#include "matrix.h"

struct vec3d;
typedef const vec3d& vecref;

struct vec3d{
	union{
		struct{mtype x,y,z;};
		mtype xyz[3];
	};

	vec3d(mtype x0=0,mtype y0=0,mtype z0=0);
	vec3d(mtype p[3]);
	vec3d(vecref v);
	vec3d(const vec3d* v);

	vec3d operator +(vecref v) const;
	vec3d operator -(vecref v) const;
	vecref operator +=(vecref v);
	vecref operator -=(vecref v);

	mtype dot(vecref v) const;
	vec3d cross(vecref v) const;

	matrix crossprodMatrix();
	matrix tensorProd(vecref v);

	void normalize();
	vec3d getNormalized() const;

	mtype magnitude() const;
	mtype magSqr() const;
	mtype invMagnitude() const;
	mtype invMagSqr() const;

	mtype operator [](int i);

	vec3d operator *(mtype a) const;
	vec3d operator /(mtype a) const;
	vecref operator *=(mtype a);
	vecref operator /=(mtype a);
	vec3d operator -() const;

	double getIncl() const;
	double getAzm() const;
	void getAngles(double* incl,double* azm) const;

	operator matrix() const;
};

mtype abs(vecref v);
mtype invsqrt(vecref v);

vec3d operator *(mtype a,vecref v);
vec3d operator *(const matrix& mat,const vec3d& v);
#endif // VEC3D_H_INCLUDED
