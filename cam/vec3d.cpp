#include "vec3d.h"
#include "../msc/functions.h"
#include <cmath>
using namespace std;

vec3d::vec3d(mtype x0,mtype y0,mtype z0):x(x0),y(y0),z(z0){
}
vec3d::vec3d(mtype p[3]):x(p[0]),y(p[1]),z(p[3]){
}
vec3d::vec3d(vecref v):x(v.x),y(v.y),z(v.z){
}
vec3d::vec3d(const vec3d* v):x(v->x),y(v->y),z(v->z){
}

vec3d vec3d::operator +(vecref v) const{
	return vec3d(x+v.x,y+v.y,z+v.z);
}
vec3d vec3d::operator -(vecref v) const{
	return vec3d(x-v.x,y-v.y,z-v.z);
}

vecref vec3d::operator +=(vecref v){
	x+=v.x;
	y+=v.y;
	z+=v.z;
	return this;
}
vecref vec3d::operator -=(vecref v){
	x-=v.x;
	y-=v.y;
	z-=v.z;
	return this;
}

mtype vec3d::dot(vecref v) const{
	return x*v.x+y*v.y+z*v.z;
}
vec3d vec3d::cross(vecref v) const{
	return vec3d(y*v.z-z*v.y,z*v.x-x*v.z,x*v.y-y*v.x);
}

matrix vec3d::crossprodMatrix(){
	mtype tmp[3][3]={
		{0,-z,y},
		{z,0,-x},
		{-y,x,0}};
	return matrix(3,3,(mtype*)tmp);//normal pointer instead of mtype** because that constructor threw errors
}
matrix vec3d::tensorProd(vecref v){
	mtype tmp[3][3]={
		{x*v.x,x*v.y,x*v.z},
		{y*v.x,y*v.y,y*v.z},
		{z*v.x,z*v.y,z*v.z}};
	return matrix(3,3,(mtype*)tmp);//normal pointer instead of mtype** because that constructor threw errors
}

void vec3d::normalize(){
	mtype d=invsqrt(this);
	x*=d;
	y*=d;
	z*=d;
}
vec3d vec3d::getNormalized() const{
	return invsqrt(this)*(*this);
}

mtype vec3d::magnitude() const{
	//return sqrt(x*x+y*y+z*z);
	return 1/invsqrt(x*x+y*y+z*z);
}
mtype vec3d::magSqr() const{
	return x*x+y*y+z*z;
}
mtype vec3d::invMagnitude() const{
	return invsqrt(x*x+y*y+z*z);
}
mtype vec3d::invMagSqr() const{
	return 1/(x*x+y*y+z*z);
}

mtype abs(vecref v){
	//return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
	return 1/invsqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}
mtype invsqrt(vecref v){
	return invsqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

mtype vec3d::operator [](int i){
	return xyz[i];//unsafe but causing an error is sometimes better (also uses less operations)
	//return xyz[(i>0)+(i>1)];//branch free access protection: if i is negative it will return the xyz[0] and if i>2 then it will return xyz[2]
}

vec3d vec3d::operator *(mtype a) const{
	return vec3d(x*a,y*a,z*a);
}
vec3d vec3d::operator /(mtype a) const{
	return vec3d(x/a,y/a,z/a);
}

vecref vec3d::operator *=(mtype a){
	x*=a;
	y*=a;
	z*=a;
	return this;
}
vecref vec3d::operator /=(mtype a){
	x/=a;
	y/=a;
	z/=a;
	return this;
}
vec3d vec3d::operator -() const{
	return vec3d(-x,-y,-z);
}

vec3d::operator matrix() const{
	mtype a[]={x,y,z};
	return matrix(3,1,a);
}

vec3d operator *(mtype a,vecref v){
	return vec3d(a*v.x,a*v.y,a*v.z);
}

double vec3d::getIncl() const{
	return asin(y*invMagnitude());
}
double vec3d::getAzm() const{
	return atan2(z,x);
}
void vec3d::getAngles(double* incl,double* azm) const{
	*incl=asin(y*invMagnitude());
	*azm=atan2(z,x);
}

badDimErr vecbaddim;
vec3d operator *(const matrix& mat,const vec3d& v){
	if(mat.rows()!=3 || mat.cols()!=3){
		throw vecbaddim;
	}

	vec3d r;
	r.x=mat(0,0)*v.x+mat(0,1)*v.y+mat(0,2)*v.z;
	r.x=mat(1,0)*v.x+mat(1,1)*v.y+mat(1,2)*v.z;
	r.x=mat(2,0)*v.x+mat(2,1)*v.y+mat(2,2)*v.z;
	return r;
}
