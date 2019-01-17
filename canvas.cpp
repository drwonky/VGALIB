/* canvas.cpp
 *
 *  Created on: Jan 5, 2019
 *      Author: pedward
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "canvas.h"
#include "sincos.h"
#include "memory.h"

canvas::~canvas()
{
	this->free();
}

void canvas::free(void)
{
	if (_buffer) delete[] _buffer;
	if (_palette) delete[] _palette;
	_buffer=NULL;
	_palette=NULL;
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

canvas::canvas(const canvas& img)
{
	_width=img._width;
	_height=img._height;
	_bgcolor=img._bgcolor;
	allocate();
	memory::fast_memcpy(_buffer,img._buffer,_width*_height);
	copypalette(img);
}

canvas& canvas::operator = (const canvas& img)
{
	_width=img._width;
	_height=img._height;
	_bgcolor=img._bgcolor;
	if (_buffer) delete[] _buffer;
	allocate();
	memory::fast_memcpy(_buffer,img._buffer,_width*_height);
	copypalette(img);

	return *this;
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

bool canvas::allocate(void)
{
	_buffer=new unsigned far char [_width*_height];

	if(_buffer) this->clear();

	return (bool)(_buffer != NULL && _palette != NULL);
}

void canvas::fill(pixel_t color)
{
	unsigned int p=_width*_height;

	while(p--) _buffer[p]=color;
}

// weighted squares color distance calculation
int32_t canvas::wcolordist(palette::pal_t *a, palette::pal_t *b)
{
	int32_t R,G,B;

	R=(int32_t)(a->r-b->r)*(int32_t)(a->r-b->r);
	G=(int32_t)(a->g-b->g)*(int32_t)(a->g-b->g);
	B=(int32_t)(a->b-b->b)*(int32_t)(a->b-b->b);
	return 2*R+4*G+3*B;
}

// squares color distance calculation
int32_t canvas::colordist(palette::pal_t *a, palette::pal_t *b)
{
	int32_t R,G,B;

	R=(int32_t)(a->r-b->r)*(int32_t)(a->r-b->r);
	G=(int32_t)(a->g-b->g)*(int32_t)(a->g-b->g);
	B=(int32_t)(a->b-b->b)*(int32_t)(a->b-b->b);
	return R+G+B;
}

// find closest matching color in image palette
unsigned char canvas::findnearestpalentry(palette::pal_t *p)
{
	int i;
	unsigned char lowindex=0;
	int32_t lowdist,distance;

	lowdist=0x2fa03; // max possible color distance

	for (i=0; i<_palette_size; i++) {
		distance=wcolordist(p,&_palette[i]);

		if (distance < lowdist) {  // sort result
			lowindex=i;
			lowdist=distance;
		}
	}

	return lowindex;
}

canvas::pixel_t canvas::lookuppalentry(palette::pal_t *p)
{
	int i;

	for (i=0; i<_palette_size; i++)
		if (_palette[i].r == p->r &&
			_palette[i].g == p->g &&
			_palette[i].b == p->b) {
				//printf("pal entry %d %d %d found at %d %02x\n",_palette[i].r,_palette[i].g,_palette[i].b,i,i);
				return i;
			}

	return _palette_size-1;
}

void canvas::drawbox(int x, int y, int width, int height, pixel_t color, bool filled)
{
	if (filled) {
		int x1,y1;
		for(y1=y;y1<y+height;y1++)
		{
			for(x1=x;x1<x+width;x1++)
			{
			   setpixel(x1,y1,color);
			}
		}
	} else {
		line(x,y,x+width,y,color);
		line(x,y,x,y+height,color);
		line(x,y+height,x+width,y+height,color);
		line(x+width,y,x+width,y+height,color);
	}
}

void canvas::line(int x1, int y1, int x2, int y2, unsigned char c)
{
		int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;

		dx=x2-x1;
		dy=y2-y1;
		dx1=abs(dx);
		dy1=abs(dy);
		px=2*dy1-dx1;
		py=2*dx1-dy1;

		if(dy1<=dx1)
		{
				if(dx>=0)
				{
						x=x1;
						y=y1;
						xe=x2;
				}
				else
				{
						x=x2;
						y=y2;
						xe=x1;
				}
				setpixel(x,y,c);
				for(i=0;x<xe;i++)
				{
						x=x+1;
						if(px<0)
						{
								px=px+2*dy1;
						}
						else
						{
								if((dx<0 && dy<0) || (dx>0 && dy>0))
								{
										y=y+1;
								}
								else
								{
										y=y-1;
								}
								px=px+2*(dy1-dx1);
						}
						setpixel(x,y,c);
				}
		}
		else
		{
				if(dy>=0)
				{
						x=x1;
						y=y1;
						ye=y2;
				}
				else
				{
						x=x2;
						y=y2;
						ye=y1;
				}
				setpixel(x,y,c);
				for(i=0;y<ye;i++)
				{
						y=y+1;
						if(py<=0)
						{
								py=py+2*dx1;
						}
						else
						{
								if((dx<0 && dy<0) || (dx>0 && dy>0))
								{
										x=x+1;
								}
								else
								{
										x=x-1;
								}
								py=py+2*(dx1-dy1);
						}
						setpixel(x,y,c);
				}
		}
}


void canvas::copyto(int x1, int y1, int x2, int y2, int width, int height)
{
	unsigned int src,rem,dest,cnt;
	ptr_t tmp;

	if (x2>_width-1 || y2>_height-1) return;

	width=x2+width > x1 ? x1-x2 : width;
	height=y2+height > y1 ? y1-y2 : height;
	src=y1*_width+x1;
	dest=y2*_width+x2;
	cnt=height;
	tmp=new unsigned char[width*height];
	rem=0;
	while(cnt) {
		memory::fast_memcpy(&tmp[rem],&_buffer[src],width);
		rem+=width;
		src+=width;
		cnt--;
	}
	cnt=height;
	rem=0;
	while(cnt) {
		memory::fast_memcpy(&_buffer[dest],&tmp[rem],width);
		dest+=_width;
		rem+=width;
		cnt--;
	}
	delete[] tmp;
}

void canvas::copyto(canvas& src, canvas& dest, int sx, int sy, int dx, int dy, int width, int height)
{
	unsigned int rem,dcnt,cnt,dw,iw;

	if (dx>dest.width()-1 || dy>dest.height()-1) return;

	iw=src.width();
	width=dx+width > dest.width() ? dest.width()-dx : width;
	height=dy+height > dest.height() ? dest.height()-dy : height;
	rem=sy*src.width()+sx;
	dcnt=dy*dest.width()+dx;
	cnt=height;
	rem=0;
	dw=dest.width();
	while(cnt) {
		memory::fast_memcpy(&dest._buffer[dcnt],&src._buffer[rem],width);
		dcnt+=dw;
		rem+=iw;
		cnt--;
	}
}

void canvas::rotate(canvas& dest, int angle)
{
	int32_t hwidth = _width / 2;
	int32_t hheight = _height / 2;
	int32_t sinma = sindeg[angle];
	int32_t cosma = cosdeg[angle];
	int32_t xt,yt,xs,ys;
	unsigned int x,y;

	for(x = 0; x < _width; x++) {
		xt = x - hwidth;
		for(y = 0; y < _height; y++) {
			yt = y - hheight;

			xs =  (cosma * xt - sinma * yt)/16384;
			xs += hwidth;
			ys =  (sinma * xt + cosma * yt)/16384;
			ys += hheight;
			if(xs >= 0 && xs < (int32_t)_width && ys >= 0 && ys < (int32_t)_height) {
				//dest._buffer[y*_width+x]=_buffer[ys*_width+xs];
				dest.setpixel(x,y,getpixel(xs,ys));
			} else {
				dest.setpixel(x,y);
				//dest._buffer[y*_width+x]=_bg;
			}
		}
	}
}

void canvas::rotate(int angle)
{
	canvas tmp = *this;

	tmp.rotate(*this,angle);

}

bool canvas::copypalette(const canvas& img)
{
	_palette_size=img.palette_size();
	if (_palette) delete[] _palette;

	_palette=new palette::pal_t[_palette_size];

	if (_palette == NULL) return false;

	memcpy(_palette,img.getpalette(),sizeof(palette::pal_t)*_palette_size);

	return true;
}

void canvas::setpalette(palette::pal_t *p)
{
        memcpy(_palette,p,sizeof(palette::pal_t)*_palette_size);
}

bool canvas::setpalette(palette::pal_type pal)
{
	_palette_size=palette::palettes[pal].palette_entries;
	_palette=new palette::pal_t[_palette_size];

	if (_palette == NULL) return false;

	memcpy(_palette,palette::palettes[pal].pal,sizeof(palette::pal_t)*_palette_size);

	return true;
}

void canvas::scale(canvas& dest, int width, int height)
{
	int x,y;

	dest.setbg(getbg());
	dest.setpalette(_palette);

	for (y=0;y<height;y++) {
		for (x=0;x<width;x++) {
			dest._buffer[y*width+x]=_buffer[(y*_height/height)*_width+(x*_width/width)];
		}
	}
}

canvas& canvas::scale(int width, int height)
{
	canvas *ret = new canvas(width,height);

	scale(*ret,width,height);

	ret->setbg(getbg());

	return *ret;
}

void canvas::scale(canvas& img)
{
	unsigned int x,y;
	int iw=img.width();
	int ih=img.height();
	setbg(img.getbg());

	for (y=0;y<_height;y++) {
		for (x=0;x<_width;x++) {
			_buffer[y*_width+x]=img._buffer[y*ih/_height*iw+x*iw/_width];
//			setpixel(x,y,img.getpixel(x*iw/_width,y*ih/h));
		}
	}
}

void canvas::drawimage(int x, int y, canvas& img, bool transparent)
{
	unsigned int rem,dest,cnt,w,h,iw;

	iw=w=img.width();
	h=img.height();

	if (x>_width-1 || y>_height-1 || x<0 || y<0) return;

	w=x+w > _width-1 ? _width-x : w;
	h=y+h > _height-1 ? _height-y : h;
	dest=y*_width+x;
	cnt=h;
	rem=0;
	while(cnt) {
		memory::fast_memcpy(&_buffer[dest],&img._buffer[rem],w);
		dest+=_width;
		rem+=iw;
		cnt--;
	}
}
