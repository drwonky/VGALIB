/*
 * matrix.cpp
 *
 *  Created on: Jan 7, 2019
 *      Author: pedward
 */

#include "matrix.h"

#ifdef __BORLANDC__
#include <iostream.h>
#include <iomanip.h>
#else
#include <iostream> // cout
#include <iomanip> // setw()

using namespace std;
#endif

#include <math.h>

template <class matrix_t>
void matrix<matrix_t>::mul(matrix& a, matrix& b, matrix& c)
{
	int col,cell,cellcnt,colcnt,rowcnt;
	matrix_t acc;

	// find the smallest matrix
	cellcnt = a.len() < b.len() ? a.len() : b.len();

	// iterate over input columns applied to matrix rows
	colcnt = a.cols();
	rowcnt = b.cols();

	for (cell=0; cell<cellcnt; cell++) {
		acc=0;
		for (col=0; col<colcnt; col++) {
			acc+=a.xy(cell/rowcnt,col) * b.xy(col,cell%rowcnt);
		}
		c[cell]=acc;  // matrix result
	}
}

template <class matrix_t>
void matrix<matrix_t>::add(matrix& a, matrix& b, matrix& c)
{
	int row,col,rowcnt,colmax;

	if (a.len() != b.len()) return;

	rowcnt=a.rows();
	colmax=a.cols();

	for(col=0;col<colmax;col++) {
		for (row=0;row<rowcnt;row++) {
			c.xy(col,row)=a.xy(col,row)+b.xy(col,row);
		}
	}
}

template <class matrix_t>
void matrix<matrix_t>::sub(matrix& a, matrix& b, matrix& c)
{
	int row,col,rowcnt,colmax;

	if (a.len() != b.len()) return;

	rowcnt=a.rows();
	colmax=a.cols();

	for(col=0;col<colmax;col++) {
		for (row=0;row<rowcnt;row++) {
			c.xy(col,row)=a.xy(col,row)-b.xy(col,row);
		}
	}
}

template <class matrix_t>
void matrix<matrix_t>::print(matrix& m)
{
	for(int y=0;y<m._rows;y++) {
//		cout<<"[";
		for(int x=0;x<m._cols;x++) {
			cout << setw(5) << m._matrix[y*m._cols+x] << " ";
		}
//		cout<<"]"<<endl;
		cout<<endl;
	}
}

#ifdef TEST
int main(void)
{
	matrix<float> a,b,c,d,e,f;
	int row,col;

	float i;

	a.init(4,4);

	i=1;
	for (row=0;row<4;row++) {
		for (col=0; col<4; col++) {
			a.xy(row,col)=i;
			i++;
		}
	}

	matrix<float>::print(a);
	cout<<"xxxx"<<endl;

	b.init(4,4);

	b.xy(0,0)=2; b.xy(0,1)=3; b.xy(0,2)=4; b.xy(0,3)=5;
	b.xy(1,0)=3; b.xy(1,1)=3; b.xy(1,2)=4; b.xy(1,3)=5;
	b.xy(2,0)=4; b.xy(2,1)=4; b.xy(2,2)=4; b.xy(2,3)=5;
	b.xy(3,0)=5; b.xy(3,1)=5; b.xy(3,2)=5; b.xy(3,3)=5;

	matrix<float>::print(b);
	cout<<"----"<<endl;
	c.init(4,4);

	matrix<float>::mul(a,b,c);

	matrix<float>::print(c);
	cout<<"----"<<endl;

	d.init(1,4);

	d.xy(0,0)=5; d.xy(0,1)=6; d.xy(0,2)=7; d.xy(0,3)=8;

	matrix<float>::print(d);
	cout<<"xxxx"<<endl;

	matrix<float>::print(b);
	cout<<"----"<<endl;

	e.init(1,4);

	matrix<float>::mul(d,b,e);

	matrix<float>::print(e);
	cout<<"----"<<endl;

	c.init(1,2);

	c.xy(0,0)=5; c.xy(0,1)=6;

	matrix<float>::print(c);
	cout<<"xxxx"<<endl;

	d.init(2,2);

	d.xy(0,0)=1; d.xy(0,1)=2;
	d.xy(1,0)=3; d.xy(1,1)=4;

	matrix<float>::print(d);
	cout<<"----"<<endl;

	e.init(1,2);

	matrix<float>::mul(c,d,e);

	matrix<float>::print(e);
	cout<<"----"<<endl;

	matrix<float>::print(d);
	cout<<"xxxx"<<endl;

	c.init(2,1);

	c.xy(0,0)=5; c.xy(1,0)=6;

	matrix<float>::print(c);
	cout<<"----"<<endl;

	e.init(2,1);

	matrix<float>::mul(d,c,e);

	matrix<float>::print(e);
	cout<<"----"<<endl;

	matrix<float> yshear(2,2);
	matrix<float> xshear(2,2);

	float angle = 45*3.14159f/180;

	yshear.xy(0,0)=1; //X = X
	yshear.xy(0,1)=tan(angle/2); // X = X * tan(angle/2)
	yshear.xy(1,0)=0; //Y = 0
	yshear.xy(1,1)=1; //Y = Y

	cout<<"yshear matrix:"<<endl;
	matrix<float>::print(yshear);

	xshear.xy(0,0)=1; // X = X;
	xshear.xy(0,1)=0; // X = 0
	xshear.xy(1,0)=-sin(angle); // Y = Y * -sin(angle);
	xshear.xy(1,1)=1; // Y = Y;

	cout<<"xshear matrix:"<<endl;
	matrix<float>::print(xshear);

	matrix<float> coords(1,2);
	matrix<float> result(1,2);
	matrix<float> result1(1,2);

	for(int y=0; y<5; y++) {
		for(int x=0; x<5; x++) {
			coords[0]=x;
			coords[1]=y;
			matrix<float>::mul(coords,yshear,result);
//			cout<<x<<" "<<y<<" ";
//			matrix<float>::print(result);
			matrix<float>::mul(result,xshear,result1);
//			cout<<x<<" "<<y<<" ";
//			matrix<float>::print(result1);
			matrix<float>::mul(result1,yshear,result);

			//cout<<"x:"<<x<<" y:"<<y<<" ";
//			cout<<x<<" "<<y<<" ";
			matrix<float>::print(result);
		}
	}
	return 0;
}

#endif
