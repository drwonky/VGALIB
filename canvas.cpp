/*
 * canvas.cpp
 *
 *  Created on: Jan 5, 2019
 *      Author: pedward
 */

#include "canvas.h"
#include <stddef.h>

canvas::~canvas()
{
	// TODO Auto-generated destructor stub
}

canvas::canvas()
{
	initvars();
}

canvas::canvas(int width, int height)
{
	initvars();
	size(width,height);
}

void canvas::initvars(void)
{
	_width=0;
	_height=0;
	_colors=0;
	_bgcolor=0;
	_buffer=NULL;
	_palette=NULL;
}
