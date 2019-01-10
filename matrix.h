/*
 * matrix.h
 *
 *  Created on: Jan 7, 2019
 *      Author: pedward
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stddef.h>
#include <string.h>

template <class matrix_t>
class matrix {
protected:
	int _rows;
	int _cols;
	matrix_t* _matrix;

public:
	matrix() { _rows=0; _cols=0; _matrix = NULL; }
	~matrix() { if (_matrix) delete[] _matrix; }
	matrix(int rows, int cols) { init(rows,cols); }
	matrix(const matrix& copy) : _rows(copy._rows), _cols(copy._cols) { _matrix = new matrix_t[_rows*_cols]; memcpy(_matrix,copy._matrix,_rows*_cols*sizeof(matrix_t)); }
	matrix& operator=(const matrix& copy) { if (this == &copy) return *this; _rows=copy._rows; _cols=copy._cols; _matrix = new matrix_t[_rows*_cols]; memcpy(_matrix,copy._matrix,_rows*_cols*sizeof(matrix_t)); return *this; }

	void init(int rows, int cols) { if (_matrix) delete[] _matrix; _rows=rows; _cols=cols; _matrix = new matrix_t[rows*cols]; clear(); }
	void clear(void) { memset(_matrix, 0 ,_rows*_cols*sizeof(matrix_t)); }

	// 2d array access pseudo operator, returns reference to array entry (assignment)
	matrix_t& xy(int row, int col) { return _matrix[row*_cols+col]; }
	// 1d array access for assignment
	matrix_t& operator[](int a) { return _matrix[a]; }
	// constant 2d array access pseudo operator, return constant reference (lookup)
	const matrix_t& xy(int row, int col) const { return _matrix[row*_cols+col]; }
	// 1d array access for reference
	const matrix_t& operator[](int a) const { return _matrix[a]; }

	const int len(void) { return _rows*_cols; }
	const int rows(void) { return _rows; }
	const int cols(void) { return _cols; }

	// operator a N b -> c
	static void mul(matrix& a, matrix& b, matrix& c);
	static void add(matrix& a, matrix& b, matrix& c);
	static void sub(matrix& a, matrix& b, matrix& c);

	static void print(matrix& m);
};



#endif /* MATRIX_H_ */
