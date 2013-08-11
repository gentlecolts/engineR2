#include "matrix.h"
#include "vec3d.h"
#include <cmath>
#include <cstdio>
using namespace std;

matrix::matrix(int row,int col,mtype fillVal):r(row),c(col){
	vals=new mtype[r*c];

	for(int i=0;i<r*c;i++){
		vals[i]=fillVal;
	}
}
matrix::matrix(int row,int col,mtype initial[]):r(row),c(col){
	vals=new mtype[r*c];

	//*
	for(int i=0;i<r*c;i++){
		vals[i]=initial[i];
	}
	/*/
	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			//printf("row:%i col:%i value:%f\n",i,j,initial[j+r*i]);
			vals[j+r*i]=initial[j+r*i];
		}
	}
	//*/
}
/*this had an error when i tried to call it from a static initialization
//also note that the above constructor works with a 2d array cast to a pointer
matrix::matrix(int row,int col,mtype** initial):r(row),c(col){
	vals=new mtype[r*c];

	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			vals[j+r*i]=initial[r][c];
		}
	}
}//*/
matrix::matrix(const matrix& m):r(m.r),c(m.c){
	vals=new mtype[r*c];

	for(int i=0;i<r*c;i++){
		vals[i]=m.vals[i];
	}
}
matrix::matrix(const matrix* m):r(m->r),c(m->c){
	vals=new mtype[r*c];

	for(int i=0;i<r*c;i++){
		vals[i]=m->vals[i];
	}
}
matrix::~matrix(){
	delete[] vals;
}

mtype& matrix::operator ()(int row,int col) const{
	return vals[col+r*row];
}

matrix& matrix::operator =(const matrix& m){
	delete[] vals;
	r=m.r;
	c=m.c;
	vals=new mtype[r*c];

	for(int i=0;i<r*c;i++){
		vals[i]=m.vals[i];
	}

	return *this;
}

badDimErr baddim;
matrix matrix::operator +(const matrix &m) const{
	if(c==m.c && r==m.r){
		matrix ret(r,c);
		for(int i=0;i<r;i++){
			for(int j=0;j<c;j++){
				ret(i,j)=(*this)(i,j)+m(i,j);
			}
		}
		return ret;
	}else{
		throw baddim;
	}
}
matrix matrix::operator -(const matrix &m) const{
	if(c==m.c && r==m.r){
		matrix ret(r,c);
		for(int i=0;i<r;i++){
			for(int j=0;j<c;j++){
				ret(i,j)=(*this)(i,j)-m(i,j);
			}
		}
		return ret;
	}else{
		throw baddim;
	}
}
matrix matrix::operator *(const matrix &m) const{
	/*
	a boring naive implementation, see
	http://en.wikipedia.org/wiki/Strassen_algorithm
	http://en.wikipedia.org/wiki/Coppersmith%E2%80%93Winograd_algorithm
	http://en.wikipedia.org/wiki/Matrix_multiplication#Algorithms_for_efficient_matrix_multiplication
	and
	http://rjlipton.wordpress.com/2011/11/29/a-breakthrough-on-matrix-product/
	for more interesting methods
	*/
	if(c==m.r){
		mtype sum;
		matrix ret(r,m.c);
		for(int i=0;i<r;i++){
			for(int j=0;j<m.c;j++){
				sum=0;
				for(int k=0;k<c;k++){
					sum+=(*this)(i,k)*m(k,j);//yay operator overloading induced syntax :D
				}
				ret(i,j)=sum;
			}
		}
		return ret;
	}else{
		throw baddim;
	}
}
matrix matrix::operator *(mtype a) const{
	matrix m((matrix*)this);
	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			m(i,j)*=a;
		}
	}
	return m;
}
matrix matrix::operator /(mtype a) const{
	matrix m((matrix*)this);
	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			m(i,j)/=a;
		}
	}
	return m;
}

///TODO: consider making these return void
matrix& matrix::operator +=(const matrix &m){
}
matrix& matrix::operator -=(const matrix &m){
}
matrix& matrix::operator *=(const matrix &m){
}
matrix& matrix::operator *=(mtype a){
	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			(*this)(i,j)*=a;
		}
	}
	return *this;
}
matrix& matrix::operator /=(mtype a){
	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			(*this)(i,j)/=a;
		}
	}
	return *this;
}

mtype matrix::get(int row,int col) const{
	return (*this)(r,c);
}
void matrix::set(int row,int col,mtype val){
	(*this)(row,col)=val;
}

mtype matrix::det() const{
}
matrix matrix::getTranspose() const{
}

int matrix::rows() const {return r;}
int matrix::cols() const {return c;}
