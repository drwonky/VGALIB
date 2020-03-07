/* canvas.cpp
 *
 *  Created on: Jan 5, 2019
 *      Author: pedward
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "canvas.h"
#include "sincos.h"
#include "memory.h"

palette::pal_t *canvas::_default_palette=(palette::pal_t *)palette::palettes[palette::RGB_PAL].pal;
int canvas::_default_palette_size=palette::palettes[palette::RGB_PAL].palette_entries;
bool canvas::_default_palette_isset=false;

canvas::~canvas()
{
	this->free();
}

void canvas::free(void)
{
	if (_buffer && !_usr_buffer) {
		delete[] _buffer;
		_buffer=NULL;
	}
	if (_palette) {
		delete[] _palette;
		_palette=NULL;
	}
	if (_pal_cache) {
		delete[] _pal_cache;
		_pal_cache=NULL;
	}
}

canvas::canvas()
{
	initvars();
}

/*
 * User supplied memory buffer ptr (for SDL surfaces)
 */
canvas::canvas(int32_t width, int32_t height, ptr_t buffer)
{
	initvars();
	if (buffer) {
		_usr_buffer = true;
		_buffer = buffer;
	}
	size(width,height);
}

canvas::canvas(const canvas& img)
{
	initvars();
	_width=img._width;
	_height=img._height;
	_bgcolor=img._bgcolor;
	copypalette(img);
	allocate();
	memory::fast_memcpy(_buffer,img._buffer,_width*_height);
}

canvas& canvas::operator = (const canvas& img)
{
	_size=img._size;
	_width=img._width;
	_height=img._height;
	_colors=img._colors;
	_bgcolor=img._bgcolor;

	if (_buffer && !_usr_buffer) {
		delete[] _buffer;
		this->_buffer = NULL;
	}

	if (img._usr_buffer) {
		_buffer = img._buffer;
		_usr_buffer=true;
	}
	copypalette(img);
	allocate();
	if (!img._usr_buffer) memory::fast_memcpy(_buffer,img._buffer,_width*_height);

	return *this;
}

unsigned char& canvas::operator [] (const int offset)
{
	return this->_buffer[offset];
}

void canvas::initvars(void)
{
	hit=miss=0;
	_size=0;
	_width=0;
	_height=0;
	_colors=0;
	_bgcolor=0;
	_buffer=NULL;
	_palette=NULL;
	_pal_cache=NULL;
	_palette_size=0;
	_usr_buffer=false;
}

bool canvas::size(int32_t width, int32_t height)
{
	if (_buffer && !_usr_buffer) {
		delete[] _buffer;
		_buffer=NULL;
	}
	_width=width;
	_height=height;

	_size=_width*_height;

	return allocate();
}

bool canvas::pow2(uint32_t in) // count how many bits are high in a number, useful for knowing if a number is the power of 2
{
	return (in & (in - 1)) == 0;
}

int canvas::bitpow(uint32_t in) // convert power of 2 decimal to bit shift count
{
	int count=0;

	for(long unsigned int i=0;i<sizeof(in)*8;i++) {
		if (in&1) break;
		count++;
		in>>=1;
	}
	return count;
}

bool canvas::allocate(void)
{
	if (!_usr_buffer) {
		_buffer=new unsigned far char [_size];
	}

	if(_buffer) this->clear();

	if(_palette == NULL) {
		copypalette(_default_palette, _default_palette_size);
	}

	return (bool)(_buffer != NULL && _palette != NULL);
}

void canvas::fill(pixel_t color)
{
#ifdef __GNUC__
	ptr_t end=_buffer+_width*_height;
	if (((_width*_height) & 3) == 0) { // dword optimization
		uint32_t c=(color<<24)|(color<<16)|(color<<8)|(color);
		uint32_t *dwend = (uint32_t *)end;
		for(uint32_t *p=(uint32_t *)_buffer;p<dwend;p++) *p=c;
	} else {
		for(ptr_t p=_buffer;p<end;p++) *p=color;
	}
#else
	for(ptr_t p=_buffer;p<_buffer+_width*_height;p++) *p=color;
#endif
}

// weighted squares color distance calculation
inline uint32_t canvas::wcolordist(palette::pal_t *a, palette::pal_t *b)
{
	int32_t R,G,B;

	R=(int32_t)(a->r-b->r)*(int32_t)(a->r-b->r);
	G=(int32_t)(a->g-b->g)*(int32_t)(a->g-b->g);
	B=(int32_t)(a->b-b->b)*(int32_t)(a->b-b->b);
	return (uint32_t)2*R+4*G+3*B;
}

// squares color distance calculation
inline uint32_t canvas::colordist(palette::pal_t *a, palette::pal_t *b)
{
	int32_t R,G,B;

	R=(int32_t)(a->r-b->r)*(int32_t)(a->r-b->r);
	G=(int32_t)(a->g-b->g)*(int32_t)(a->g-b->g);
	B=(int32_t)(a->b-b->b)*(int32_t)(a->b-b->b);
	return (uint32_t)R+G+B;
}

// find closest matching color in image palette
unsigned char canvas::findnearestpalentry(palette::pal_t *p)
{
	int i;
	unsigned char lowindex=0;
	uint32_t lowdist,distance;

	if (_pal_cache != NULL) {
		if ((i = lookuppalcache(p)) != -1)
		{
			hit++;
			return (unsigned char)i;
		} else {
//			cout<<"miss r "<<(int)p->r<<" g "<<(int)p->g<<" b "<<(int)p->b<<endl;
			miss++;
		}
	} else {
		_pal_cache = new palette::pal_t[_palette_size];
		for (int i=0; i<_palette_size; i++)
			_pal_cache[i].r=_pal_cache[i].g=_pal_cache[i].b=0;
	}


	lowdist=0x30F201; // max possible color distance

	for (i=0; i<_palette_size; i++) {
		distance=wcolordist(p,&_palette[i]);

		if (distance < lowdist) {  // sort result
			lowindex=i;
			lowdist=distance;
		}
	}

	_pal_cache[lowindex].r=p->r;
	_pal_cache[lowindex].g=p->g;
	_pal_cache[lowindex].b=p->b;
	return (unsigned char)lowindex;
}

int canvas::lookuppalcache(palette::pal_t *p)
{
	int i;

	for (i=0; i<_palette_size; i++)
		if (_pal_cache[i].r == p->r &&
			_pal_cache[i].g == p->g &&
			_pal_cache[i].b == p->b) {
				return i;
			}

	return -1;
}

canvas::pixel_t canvas::lookuppalentry(palette::pal_t *p)
{
	int i;

	for (i=0; i<_palette_size; i++)
		if (_palette[i].r == p->r &&
			_palette[i].g == p->g &&
			_palette[i].b == p->b) {
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
	int32_t x,y;

	for(x = 0; x < _width; x++) {
		xt = x - hwidth;
		for(y = 0; y < _height; y++) {
			yt = y - hheight;

			xs =  (cosma * xt - sinma * yt)>>14;
			xs += hwidth;
			ys =  (sinma * xt + cosma * yt)>>14;
			ys += hheight;
			if(xs >= 0 && xs < (int32_t)_width && ys >= 0 && ys < (int32_t)_height) {
				//dest._buffer[y*_width+x]=_buffer[ys*_width+xs];
				dest.setpixel(x,y,getpixel(xs,ys));
			} else {
				dest.setpixel(x,y); // @suppress("Ambiguous problem")
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
	if (_palette == img._palette) return true; // default palette is statically allocated

	_palette_size=img.palette_size();
	if (_palette) delete[] _palette;

	_palette=new palette::pal_t[_palette_size];

	if (_palette == NULL) return false;

	memcpy(_palette,img.getpalette(),sizeof(palette::pal_t)*_palette_size);

	return true;
}

bool canvas::copypalette(palette::pal_t *p, int size)
{
	_palette_size=size;

	if (_palette) delete[] _palette;

	_palette=new palette::pal_t[_palette_size];

	if (_palette == NULL) return false;

	memcpy(_palette,p,sizeof(palette::pal_t)*_palette_size);

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

void canvas::setdefpalette(palette::pal_t *p, int size)
{
	_default_palette_size=size;

	if (_default_palette_isset == true) delete[] _default_palette;

	_default_palette=new palette::pal_t[_default_palette_size];

	if (_default_palette == NULL) {
		_default_palette_size=0;
		return;
	}

	memcpy(_default_palette,p,sizeof(palette::pal_t)*_default_palette_size);

	_default_palette_isset=true;
}

bool canvas::setdefpalette(palette::pal_type pal)
{
	_default_palette_size=palette::palettes[pal].palette_entries;

	if (_default_palette_isset == true) delete[] _default_palette;

	_default_palette=new palette::pal_t[_default_palette_size];

	if (_default_palette == NULL) return false;

	memcpy(_default_palette,palette::palettes[pal].pal,sizeof(palette::pal_t)*_default_palette_size);

	_default_palette_isset=true;

	return true;
}

// general purpose scale/draw this image onto another image
void canvas::scale(canvas& dest, int width, int height)
{
	int x,y;

	dest.setbg(getbg());
	dest.setpalette(_palette);

	for (y=0;y<height;y++) {
		for (x=0;x<width;x++) {
//			dest._buffer[y*width+x]=_buffer[(y*_height/height)*_width+(x*_width/width)];
			dest.setpixel(x,y,getpixel(x*_width/width,y*_height/_height));
		}
	}
}

canvas canvas::scale(int width, int height)
{
	canvas ret(width,height);

	scale(ret,width,height);

	ret.setbg(getbg());

	return ret;
}

// nearest neighbor scaling, this is the fastest path of all of the scale methods
void canvas::scale(canvas& img)
{
	int32_t x,y;
	int32_t src_width=img.width();
	int32_t src_height=img.height();
	int32_t tx,ty;
	int32_t scale_factor=(_width/src_width);

	setbg(img.getbg());

	// if the scale factor is 2 or greater and a power of 2..4..8..16
	if (_width>src_width && _height>src_height &&  // redundant checks that quickly filter out scaling up from scaling down, allows compiler to bail quicker
		(scale_factor&1) == 0 && // must be 2 or greater
		pow2(scale_factor) // is exactly a power of 2
//			(_width%src_width) == 0 && // no fractional scaling
//			(scale_factor == (_height/src_height)) && // x and y scaling are same
	) {
		int32_t scale_shift=bitpow(scale_factor); // convert decimal scale factor to bit shift

		for (y=0;y<_height;y+=scale_factor) {
			ty=y>>scale_shift;
			for (x=0;x<_width;x++) {
				tx=x>>scale_shift; // power of 2 scaling
				setpixel(x,y,img.getpixel(tx,ty));
			}
			for (int32_t rowcnt=y;rowcnt<y+scale_factor;rowcnt++) {
				memory::fast_memcpy(&_buffer[rowcnt*_width],&_buffer[y*_width],_width);
			}
		}
	} else {
		int32_t x_rat=((src_width<<16)/_width)+1;
		int32_t y_rat=((src_height<<16)/_height)+1;

		for (y=0;y<_height;y++) {
			ty=((y*y_rat)>>16);
			for (x=0;x<_width;x++) {
				tx=(x*x_rat)>>16;
				setpixel(x,y,img.getpixel(tx,ty));
			}
		}
	}
}

// TODO 2x smooth zoom
void canvas::scaleDCCI(canvas& img)
{
	/*
	uint32_t x,y;
	int32_t src_width=img.width();
	int32_t src_height=img.height();

	setbg(img.getbg());
	clear();
	size(src_width<<1,src_height<<1);

	for (y=0;y<src_height;y++) {
		for (x=0;x<src_width;x++) {
			setpixel(x<<1,y<<1,img.getpixel(x,y));
		}
	}

	int d1,d2;

	for (y=0;y<_height;y++) {
		for (x=0;x<_width;x++) {

//			setpixel(x,y,img.getpixel(x*iw/_width,y*ih/_height));
		}
	}
	*/
}

void canvas::drawimage(int x, int y, canvas& img, bool transparent)
{
	int32_t w,h,iw;
	ptr_t src,dest;

	// if image is off screen, abort
	if (x>_width-1 || y>_height-1 || x<0 || y<0) return;

	dest=_buffer+(y*_width+x);
	src=img._buffer;

	h=img.height();
	w=iw=img.width();

	// This implements height clipping
	if (y+h > _height-1) h = _height-y;

	// This implements width clipping
	if (x+w > _width-1) {
		w = _width-x;
	} else {
		// If the source is the same width as the dest, we can just copy a contiguous block of data, can't do this with clipping
		if (!transparent && x==0 && w==_width) {
//			memory::blit(dest,rem, h < (_height - y) ? img._size : (_height - y) * _width);  // this clips height
			memory::blit(dest,src, h * w);
			return;
		}
	}

	if (transparent) {
		unsigned char mask=img.getbg();
		while(h) {
			memory::mask_memcpy(dest,src,w,mask);	// width is clipped to w
			dest+=_width;
			src+=iw;								// if clipping, skip 1 row length
			h--;
		}
	} else {
		while(h) {
			memory::fast_memcpy(dest,src,w);
			dest+=_width;
			src+=iw;
			h--;
		}
	}
}

/*
 * Source image wider or taller than destination, allows for panning or sprite sheets
 */
void canvas::drawimage(int x, int y, int sx, int sy, int width, int height, canvas& img, bool transparent)
{
	int32_t w,h,iw;
	ptr_t src,dest;

	iw=img.width();
	w=width;
	h=height;

	if (x>_width-1 || y>_height-1 || x<0 || y<0) return;

	dest=_buffer+(y*_width+x);
	src=img._buffer+(sy*img.width()+sx);

	// This implements clipping
	w=x+w > _width-1 ? _width-x : w;
	h=y+h > _height-1 ? _height-y : h;

	if (transparent) {
		unsigned char mask=img.getbg();
		while(h) {
			memory::mask_memcpy(dest,src,w,mask);
			dest+=_width;
			src+=iw;
			h--;
		}
	} else {
		while(h) {
			memory::fast_memcpy(dest,src,w);
			dest+=_width;
			src+=iw;
			h--;
		}
	}
}
