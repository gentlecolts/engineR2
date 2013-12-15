#include "quat.h"
#include "matrix.h"
#include "vec3d.h"
#include "../msc/functions.h"
#include <cmath>
using namespace std;

quat::quat():a(0),b(0),c(0),d(0){
}
quat::quat(double a0,double b0,double c0,double d0):a(a0),b(b0),c(c0),d(d0){
}

quat::quat(double t,vecref v){
	setFromAngleVector(t,v);
}

void quat::setFromAngleVector(double t,vecref v){
	const double s=sin(t/2);
	a=cos(t/2);
	b=v.x*s;
	c=v.y*s;
	d=v.z*s;

	//printf("angle: %f\nvector: <%f,%f,%f>\nq: <%f,%f,%f,%f>\nmagnitude^2: %f\n\n",t,v.x,v.y,v.z,a,b,c,d,a*a+b*b+c*c+d*d);

	//normalize();
}

quat quat::operator +(quatref q) const{
	return quat(a+q.a,b+q.b,c+q.c,d+q.d);
}
quat quat::operator -(quatref q) const{
	return quat(a-q.a,b-q.b,c-q.c,d-q.d);
}
quat quat::operator *(quatref q) const{
	return quat(
			a*q.a-b*q.b-c*q.c-d*q.d,
			a*q.b+b*q.a+c*q.d-d*q.c,
			a*q.c-b*q.d+c*q.a+d*q.b,
			a*q.d+b*q.c-c*q.b+d*q.a);
}
quat quat::operator /(quatref q) const{
	return (*this)*q.inv();
}

quat quat::operator *(double q) const{
	return quat(a*q,b*q,c*q,d*q);
}
quat quat::operator /(double q) const{
	return quat(a/q,b/q,c/q,d/q);
}

quat quat::operator *(vecref v) const{
	return quat(
			-b*v.x-c*v.y-d*v.z,
			a*v.x+c*v.z-d*v.y,
			a*v.y-b*v.z+d*v.x,
			a*v.z+b*v.y-c*v.x);
}
quat operator *(vecref v,quatref q){
	return quat(
			-v.x*q.b-v.y*q.c-v.z*q.d,
			v.x*q.a+v.y*q.d-v.z*q.c,
			-v.x*q.d+v.y*q.a+v.z*q.b,
			v.x*q.c-v.y*q.b+v.z*q.a);
}

vec3d quat::applyTo(vecref v) const{
	/*
	return quat(
			-b*v.x-c*v.y-d*v.z,
			a*v.x+c*v.z-d*v.y,
			a*v.y-b*v.z+d*v.x,
			a*v.z+b*v.y-c*v.x)
			*
			quat(a,-b,-c,-d);
	*/
	/*
	#if 0
	return quat(
		(-b*v.x-c*v.y-d*v.z)*a-(a*v.x+c*v.z-d*v.y)*(-b)-(a*v.y-b*v.z+d*v.x)*(-c)-(a*v.z+b*v.y-c*v.x)*(-d),
		(-b*v.x-c*v.y-d*v.z)*(-b)+(a*v.x+c*v.z-d*v.y)*a+(a*v.y-b*v.z+d*v.x)*(-d)-(a*v.z+b*v.y-c*v.x)*(-c),
		(-b*v.x-c*v.y-d*v.z)*(-c)-(a*v.x+c*v.z-d*v.y)*(-d)+(a*v.y-b*v.z+d*v.x)*a+(a*v.z+b*v.y-c*v.x)*(-b),
		(-b*v.x-c*v.y-d*v.z)*(-d)+(a*v.x+c*v.z-d*v.y)*(-c)-(a*v.y-b*v.z+d*v.x)*(-b)+(a*v.z+b*v.y-c*v.x)*a);
	#else
	return quat(
		(-b*v.x-c*v.y-d*v.z)*a+(a*v.x+c*v.z-d*v.y)*b+(a*v.y-b*v.z+d*v.x)*c+(a*v.z+b*v.y-c*v.x)*d,
		(-b*v.x-c*v.y-d*v.z)*(-b)+(a*v.x+c*v.z-d*v.y)*a+(a*v.y-b*v.z+d*v.x)*(-d)+(a*v.z+b*v.y-c*v.x)*c,
		(-b*v.x-c*v.y-d*v.z)*(-c)+(a*v.x+c*v.z-d*v.y)*d+(a*v.y-b*v.z+d*v.x)*a+(a*v.z+b*v.y-c*v.x)*(-b),
		(-b*v.x-c*v.y-d*v.z)*(-d)+(a*v.x+c*v.z-d*v.y)*(-c)+(a*v.y-b*v.z+d*v.x)*b+(a*v.z+b*v.y-c*v.x)*a);
	#endif
	/*/
	#if 0
	return quat(
		//-b*v.x*a - c*v.y*a - d*v.z*a + a*v.x*b + c*v.z*b - d*v.y*b + a*v.y*c - b*v.z*c + d*v.x*c + a*v.z*d + b*v.y*d - c*v.x*d,
		//-a*b*v.x - a*c*v.y - a*d*v.z + a*b*v.x + b*c*v.z - b*d*v.y + a*c*v.y - b*c*v.z + c*d*v.x + a*d*v.z + b*d*v.y - c*d*v.x,
		//-a*b*v.x + a*b*v.x - a*c*v.y + a*c*v.y - a*d*v.z + a*d*v.z + b*c*v.z - b*c*v.z - b*d*v.y + b*d*v.y + c*d*v.x - c*d*v.x,
		//0 - 0 - 0 + 0 - 0 + 0,
		0,
		//-b*v.x*(-b) - c*v.y*(-b) - d*v.z*(-b) + a*v.x*a + c*v.z*a - d*v.y*a + a*v.y*(-d) - b*v.z*(-d) + d*v.x*(-d) + a*v.z*c + b*v.y*c - c*v.x*c,
		//b*v.x*b + c*v.y*b + d*v.z*b + a*v.x*a + c*v.z*a - d*v.y*a - a*v.y*d + b*v.z*d - d*v.x*d + a*v.z*c + b*v.y*c - c*v.x*c,
		//b*b*v.x + b*c*v.y + b*d*v.z + a*a*v.x + a*c*v.z - a*d*v.y - a*d*v.y + b*d*v.z - d*d*v.x + a*c*v.z + b*c*v.y - c*c*v.x,
		//a*a*v.x + a*c*v.z + a*c*v.z - a*d*v.y - a*d*v.y + b*b*v.x + b*c*v.y + b*c*v.y + b*d*v.z + b*d*v.z - c*c*v.x - d*d*v.x,
		//a*a*v.x + 2*a*c*v.z - 2*a*d*v.y + b*b*v.x + 2*b*c*v.y + 2*b*d*v.z - c*c*v.x - d*d*v.x,
		//(a*a + b*b - c*c - d*d)*v.x + 2*(a*c*v.z - a*d*v.y + b*c*v.y + b*d*v.z),
		(a*a+b*b-c*c-d*d)*v.x + 2*((b*c - a*d)*v.y + (a*c + b*d)*v.z),
		//-b*v.x*(-c) - c*v.y*(-c) - d*v.z*(-c) + a*v.x*d + c*v.z*d - d*v.y*d + a*v.y*a - b*v.z*a + d*v.x*a + a*v.z*(-b) + b*v.y*(-b) - c*v.x*(-b),
		//b*v.x*c + c*v.y*c + d*v.z*c + a*v.x*d + c*v.z*d - d*v.y*d + a*v.y*a - b*v.z*a + d*v.x*a - a*v.z*b - b*v.y*b + c*v.x*b,
		//b*c*v.x + c*c*v.y + c*d*v.z + a*d*v.x + c*d*v.z - d*d*v.y + a*a*v.y - a*b*v.z + a*d*v.x - a*b*v.z - b*b*v.y + b*c*v.x,
		//a*a*v.y - a*b*v.z - a*b*v.z + a*d*v.x + a*d*v.x - b*b*v.y + b*c*v.x + b*c*v.x + c*c*v.y + c*d*v.z + c*d*v.z - d*d*v.y,
		//a*a*v.y - 2*a*b*v.z + 2*a*d*v.x - b*b*v.y + 2*b*c*v.x + c*c*v.y + 2*c*d*v.z - d*d*v.y,
		(a*a-b*b+c*c-d*d)*v.y + 2*((c*d - a*b)*v.z + (a*d + b*c)*v.x),
		//-b*v.x*(-d) - c*v.y*(-d) - d*v.z*(-d) + a*v.x*(-c) + c*v.z*(-c) - d*v.y*(-c) + a*v.y*b - b*v.z*b + d*v.x*b + a*v.z*a + b*v.y*a - c*v.x*a);
		//b*v.x*d + c*v.y*d + d*v.z*d - a*v.x*c - c*v.z*c + d*v.y*c + a*v.y*b - b*v.z*b + d*v.x*b + a*v.z*a + b*v.y*a - c*v.x*a
		//b*d*v.x + c*d*v.y + d*d*v.z - a*c*v.x - c*c*v.z + c*d*v.y + a*b*v.y - b*b*v.z + b*d*v.x + a*a*v.z + a*b*v.y - a*c*v.x
		//a*a*v.z + a*b*v.y + a*b*v.y - a*c*v.x - a*c*v.x - b*b*v.z + b*d*v.x + b*d*v.x - c*c*v.z + c*d*v.y + c*d*v.y + d*d*v.z
		//a*a*v.z + 2*a*b*v.y - 2*a*c*v.x - b*b*v.z + 2*b*d*v.x - c*c*v.z + 2*c*d*v.y + d*d*v.z
		(a*a-b*b-c*c+d*d)*v.z + 2*((b*d - a*c)*v.x + (a*b + c*d)*v.y)
		);
	#else
	#if 1
	return vec3d(
		//*
		(a*a+b*b-c*c-d*d)*v.x + 2*((b*c - a*d)*v.y + (a*c + b*d)*v.z),
		(a*a-b*b+c*c-d*d)*v.y + 2*((c*d - a*b)*v.z + (a*d + b*c)*v.x),
		(a*a-b*b-c*c+d*d)*v.z + 2*((b*d - a*c)*v.x + (a*b + c*d)*v.y)
		/*/
		a*a*v.x+b*b*v.x-c*c*v.x-d*d*v.x + 2*(b*c*v.y - a*d*v.y + a*c*v.z + b*d*v.z),
		a*a*v.y-b*b*v.y+c*c*v.y-d*d*v.y + 2*(c*d*v.z - a*b*v.z + a*d*v.x + b*c*v.x),
		a*a*v.z-b*b*v.z-c*c*v.z+d*d*v.z + 2*(b*d*v.x - a*c*v.x + a*b*v.y + c*d*v.y)
		//*/
	);
	#else
	vec3d r(
		(a*a+b*b-c*c-d*d)*v.x + 2*((b*c - a*d)*v.y + (a*c + b*d)*v.z),
		(a*a-b*b+c*c-d*d)*v.y + 2*((c*d - a*b)*v.z + (a*d + b*c)*v.x),
		(a*a-b*b-c*c+d*d)*v.z + 2*((b*d - a*c)*v.x + (a*b + c*d)*v.y)
	);
	printf("|q|^2: %f\nr^2 before: %f\nr^2 after: %f\n\n",magSqr(),v.magSqr(),r.magSqr());
	//return r*invsqrt(r.magSqr()/v.magSqr());
	return r;
	#endif
	#endif
	//*/
}

quat& quat::operator +=(quatref q){
	a+=q.a;
	b+=q.b;
	c+=q.c;
	d+=q.d;
	return *this;
}
quat& quat::operator -=(quatref q){
	a-=q.a;
	b-=q.b;
	c-=q.c;
	d-=q.d;
	return *this;
}
quat& quat::operator *=(quatref q){
	return *this=(*this)*q;
}
quat& quat::operator /=(quatref q){
	return *this=(*this)*q.inv();
}

quat& quat::operator *=(double q){
	a*=q;
	b*=q;
	c*=q;
	d*=q;
	return *this;
}
quat& quat::operator /=(double q){
	a/=q;
	b/=q;
	c/=q;
	d/=q;
	return *this;
}

quat quat::conj() const{
	return quat(a,-b,-c,-d);
}
quat quat::inv() const{
	return conj()/magSqr();
}

double quat::magnitude() const{
	return 1/invsqrt(a*a+b*b+c*c+d*d);
}
double quat::invmag() const{
	return invsqrt(a*a+b*b+c*c+d*d);
}
double quat::magSqr() const{
	return a*a+b*b+c*c+d*d;
}
double quat::invmagSqr() const{
	return 1/(a*a+b*b+c*c+d*d);
}

quat quat::getNormalized() const{
	const double q=invmag();
	return quat(a*q,b*q,c*q,d*q);
}
void quat::normalize(){
	//const double q=invsqrt(a*a+b*b+c*c+d*d);
	const double q=1/sqrt(a*a+b*b+c*c+d*d);
	a*=q;
	b*=q;
	c*=q;
	d*=q;
}

quat::operator matrix() const{
	mtype tmp[9]={
		a*a+b*b-c*c-d*d,2*(b*c-a*d),2*(b*d+a*c),
		2*(b*c+a*d),a*a-b*b+c*c-d*d,2*(c*d-a*b),
		2*(b*d-a*c),2*(c*d+a*b),a*a-b*b-c*c+d*d};
	return matrix(3,3,tmp);
}

double abs(quatref q){
	return q.magnitude();
}
quat conj(quatref q){
	return q.conj();
}

quat operator *(double a,quatref q){
	return q*a;
}
quat operator /(double a,quatref q){
	return a*q.inv();
}
