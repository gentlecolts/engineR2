#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED
#include <exception>
typedef double mtype;

#define NULL __null

class matrix{
private:
	int r,c;
	mtype* vals;
public:
	matrix(int row,int col,mtype fillVal=0);
	matrix(int row,int col,mtype* inital);
	//matrix(int row,int col,mtype** inital);//removed until i find a reason for it or am compelled to delete it
	matrix(const matrix& m);
	matrix(const matrix* m);
	virtual ~matrix();

	virtual mtype& operator ()(int row,int col) const;

	virtual matrix& operator =(const matrix& m);

	virtual matrix operator +(const matrix &m) const;
	virtual matrix operator -(const matrix &m) const;
	virtual matrix operator *(const matrix &m) const;
	virtual matrix operator *(mtype a) const;
	virtual matrix operator /(mtype a) const;

	virtual matrix& operator +=(const matrix &m);
	virtual matrix& operator -=(const matrix &m);
	virtual matrix& operator *=(const matrix &m);
	virtual matrix& operator *=(mtype a);
	virtual matrix& operator /=(mtype a);

	/*why did i put any of these here?
	virtual matrix add(matrix* m);
	virtual matrix subtract(matrix* m);
	virtual matrix cross(matrix* m);
	virtual mtype dot(matrix* m);
	virtual void inc(matrix* m);
	virtual void dec(matrix* m);
	virtual void crossWith(matrix* m);
	*/

	virtual mtype get(int row,int col) const;
	virtual void set(int row,int col,mtype val);

	virtual mtype det() const;
	virtual matrix getTranspose() const;

	virtual int rows() const;
	virtual int cols() const;
};

class sqrMatrix:public matrix{
	sqrMatrix(int n,mtype inital[9]=NULL);
	sqrMatrix(const sqrMatrix& m);
	sqrMatrix(sqrMatrix* m);

	virtual mtype det();
};

class badDimErr: public std::exception{
  virtual const char* what() const throw(){
    return "Operation between matricies of incompatible dimensions";
  }
};
#endif // MATRIX_H_INCLUDED
