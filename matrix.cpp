/*
 * matrix.cpp
 *
 *  Created on: Jan 7, 2019
 *      Author: pedward
 */

#include "matrix.h"

#ifdef __BORLANDC__
#include <iostream.h>
#else
#include <iostream> // std::cout
#include <iomanip> // std::setw()
#endif

void matrix::mul(matrix& a, matrix& b, matrix& c)
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

void matrix::add(matrix& a, matrix& b, matrix& c)
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

void matrix::sub(matrix& a, matrix& b, matrix& c)
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

void matrix::print(matrix& m)
{
	for(int y=0;y<m._rows;y++) {
		std::cout<<"[";
		for(int x=0;x<m._cols;x++) {
			std::cout << std::setw(5) << m._matrix[y*m._cols+x] << " ";
		}
		std::cout<<"]"<<std::endl;
	}
}

#ifdef TEST
int main(void)
{
	matrix a,b,c,d,e,f;
	int row,col;

	matrix_t i;

	a.init(4,4);

	i=1;
	for (row=0;row<4;row++) {
		for (col=0; col<4; col++) {
			a.xy(row,col)=i;
			i++;
		}
	}

	matrix::print(a);
	std::cout<<"xxxx"<<std::endl;

	b.init(4,4);

	b.xy(0,0)=2; b.xy(0,1)=3; b.xy(0,2)=4; b.xy(0,3)=5;
	b.xy(1,0)=3; b.xy(1,1)=3; b.xy(1,2)=4; b.xy(1,3)=5;
	b.xy(2,0)=4; b.xy(2,1)=4; b.xy(2,2)=4; b.xy(2,3)=5;
	b.xy(3,0)=5; b.xy(3,1)=5; b.xy(3,2)=5; b.xy(3,3)=5;

	matrix::print(b);
	std::cout<<"----"<<std::endl;
	c.init(4,4);

	matrix::mul(a,b,c);

	matrix::print(c);
	std::cout<<"----"<<std::endl;

	d.init(1,4);

	d.xy(0,0)=5; d.xy(0,1)=6; d.xy(0,2)=7; d.xy(0,3)=8;

	matrix::print(d);
	std::cout<<"xxxx"<<std::endl;

	matrix::print(b);
	std::cout<<"----"<<std::endl;

	e.init(1,4);

	matrix::mul(d,b,e);

	matrix::print(e);
	std::cout<<"----"<<std::endl;

	c.init(1,2);

	c.xy(0,0)=5; c.xy(0,1)=6;

	matrix::print(c);
	std::cout<<"xxxx"<<std::endl;

	d.init(2,2);

	d.xy(0,0)=1; d.xy(0,1)=2;
	d.xy(1,0)=3; d.xy(1,1)=4;

	matrix::print(d);
	std::cout<<"----"<<std::endl;

	e.init(1,2);

	matrix::mul(c,d,e);

	matrix::print(e);
	std::cout<<"----"<<std::endl;

	matrix::print(d);
	std::cout<<"xxxx"<<std::endl;

	c.init(2,1);

	c.xy(0,0)=5; c.xy(1,0)=6;

	matrix::print(c);
	std::cout<<"----"<<std::endl;

	e.init(2,1);

	matrix::mul(d,c,e);

	matrix::print(e);
	std::cout<<"----"<<std::endl;

}

#endif
