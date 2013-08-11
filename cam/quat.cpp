#include "quat.h"
#include "matrix.h"
#include "vec3d.h"
#include "../msc/functions.h"
#include <cmath>
using namespace std;

quat::quat(double a0,double x,double y,double z):a(a0),b(x),c(y),d(z){
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
	const double q=invmag();
	a*=q;
	b*=q;
	c*=q;
	d*=q;
}

quat::operator matrix() const{
	double tmp[9]={
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
