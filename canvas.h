/*
 * canvas.h
 *
 *  Created on: Jan 5, 2019
 *      Author: pedward
 */

#ifndef CANVAS_H_
#define CANVAS_H_

#include "types.h"
#include "palettes.h"

#ifndef __BORLANDC__
using namespace std;
#include <iostream>
#else
#include <iostream.h>
#endif

class canvas {
public:
	typedef unsigned char pixel_t;
	typedef char * string_t;
	ptr_t _buffer;

protected:
	int _width;
	int _height;
	pixel_t _colors;
	pixel_t _bgcolor;

	palette::pal_t *_palette;
	int _palette_size;

private:
	void initvars(void);	// c++98 does not support delegating constructors

public:
	virtual ~canvas();
	canvas();
	canvas(int width, int height);
	canvas(const canvas& img);
	canvas& operator = (const canvas& img);
	virtual void free(void);
	virtual bool copybuffer(ptr_t *src) { return true; }
	virtual ptr_t getbuffer(void) { return _buffer; }
	virtual int width(void) { return _width; }
	virtual int height(void) { return _height; }
	virtual void setpixel(int x, int y) { _buffer[y*_width+x]=_bgcolor; }
	virtual void setpixel(int x, int y, pixel_t color) { _buffer[y*_width+x]=color; }
	virtual pixel_t getpixel(int x, int y) { return _buffer[y*_width+x]; }
	virtual bool allocate(void);
	virtual void drawbox(int x, int y, int width, int height, pixel_t color, bool filled = true);
	virtual void drawimage(int x, int y, canvas& img, bool transparent = false);
	virtual void drawsprite(int x, int y, canvas& img) {;}
	virtual void copyto(int x1, int y1, int x2, int y2, int width, int height);
	virtual void copyto(canvas& src, canvas& dest, int sx, int sy, int dx, int dy, int width, int height);
	virtual void line(int x1, int y1, int x2, int y2, pixel_t color);
	virtual void clear(void) { fill(_bgcolor); }
	virtual void fill(pixel_t color);
	virtual void drawtext(string_t str, int x, int y, pixel_t color) {;}
	virtual bool setpalette(palette::pal_type pal);
	virtual void setpalette(palette::pal_t *pal);
	virtual const palette::pal_t *getpalette(void) const { return _palette; }
	virtual int palette_size(void) const { return _palette_size; }
	virtual palette::pal_t getpalentry(pixel_t i) { return (palette::pal_t)_palette[i]; }
	virtual bool copypalette(const canvas& img);
	virtual int32_t wcolordist(palette::pal_t *a, palette::pal_t *b);
	virtual int32_t colordist(palette::pal_t *a, palette::pal_t *b);
	virtual pixel_t lookuppalentry(palette::pal_t *p);
	virtual pixel_t findnearestpalentry(palette::pal_t *p);
	virtual bool size(int width, int height) { if (_buffer) delete[] _buffer; _width=width; _height=height; return allocate(); }
	virtual void setbg(pixel_t background){ _bgcolor = background; }
	virtual pixel_t getbg(void) { return _bgcolor; }
	virtual void rotate(int x, int y, int width, int height, int angle) {;}
	virtual void rotate(canvas& dest, int angle);
	virtual void rotate(int angle);
	virtual void scale(canvas& dest, int width, int height);
	virtual canvas& scale(int width, int height);
	virtual void scale(canvas& img);

};

#endif /* CANVAS_H_ */
