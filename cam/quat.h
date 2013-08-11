#ifndef QUAT_H_INCLUDED
#define QUAT_H_INCLUDED

class matrix;
struct quat;

typedef const quat& quatref;

struct vec3d;
typedef const vec3d& vecref;

struct quat{
	double a,b,c,d;

	quat(double a0=0,double x=0,double y=0,double z=0);
	quat(double t,vecref v);

	void setFromAngleVector(double t,vecref v);

	quat operator +(quatref q) const;
	quat operator -(quatref q) const;
	quat operator *(quatref q) const;
	quat operator /(quatref q) const;

	quat operator *(double q) const;
	quat operator /(double q) const;

	quat& operator +=(quatref q);
	quat& operator -=(quatref q);
	quat& operator *=(quatref q);
	quat& operator /=(quatref q);

	quat& operator *=(double q);
	quat& operator /=(double q);

	quat conj() const;
	quat inv() const;

	double magnitude() const;
	double invmag() const;
	double magSqr() const;
	double invmagSqr() const;

	quat getNormalized() const;
	void normalize();

	operator matrix() const;
};

double abs(quatref q);
quat conj(quatref q);

quat operator *(double a,quatref q);
quat operator /(double a,quatref q);

#endif // QUAT_H_INCLUDED
