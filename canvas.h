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
	const int MAX_PLANES = 4;
	ptr_t _plane[MAX_PLANES];
	unsigned int _size;

protected:
	int32_t _width;
	int32_t _height;
	pixel_t _colors;
	pixel_t _bgcolor;
	pixel_t _bpp;
	pixel_t _planes;

	palette::pal_t *_palette;
	palette::pal_t *_pal_cache;
	int _palette_size;

	bool _usr_buffer;

private:
	static palette::pal_t *_default_palette;
	static int _default_palette_size;
	static bool _default_palette_isset;  // We use static initialization to give a sane palette to new instances, but we want to be able to override the static reference.

protected:
	virtual void initvars(void);	// c++98 does not support delegating constructors

public:
	virtual ~canvas();
	canvas();
	canvas(int32_t width, int32_t height, ptr_t buffer = NULL);
	canvas(int32_t width, int32_t height, pixel_t bpp);
	canvas(const canvas& img);
	canvas& operator = (const canvas& img);
	unsigned char& operator [] (const int offset);
	bool pow2(uint32_t in);
	int bitpow(uint32_t in);
	virtual void free(void);
	virtual void freepalette(void);
	virtual void freebuffers(void);
	virtual ptr_t getbuffer(void) const
		{ return &_plane; }
	virtual int32_t width(void) const
		{ return _width; }
	virtual int32_t height(void) const
		{ return _height; }
	virtual void setpixel(int x, int y) { _buffer[y*_width+x]=_bgcolor; }
	virtual void setpixel(int x, int y, pixel_t color) { _buffer[y*_width+x]=color; }
	virtual pixel_t getpixel(int x, int y) const
		{ return _buffer[y*_width+x]; }
	virtual bool allocate(void);
	virtual void drawbox(int x, int y, int width, int height, pixel_t color, bool filled = true);
	virtual void drawimage(int x, int y, canvas& img, bool transparent = false);
	virtual void drawimage(int x, int y, int sx, int sy, int width, int height, canvas& img, bool transparent = false);
	virtual void drawsprite(int x, int y, canvas& img) { this->drawimage(x,y,img,true); }
	virtual void copyto(int x1, int y1, int x2, int y2, int width, int height);
	virtual void copyto(canvas& src, canvas& dest, int sx, int sy, int dx, int dy, int width, int height);
	virtual void line(int x1, int y1, int x2, int y2, pixel_t color);
	virtual void clear(void) { fill(_bgcolor); }
	virtual void fill(pixel_t color);
	virtual void drawtext(string_t str, int x, int y, pixel_t color) {;}
	virtual bool setpalette(palette::pal_type pal);
	virtual void setpalette(palette::pal_t *pal);
	static bool setdefpalette(palette::pal_type pal);
	static void setdefpalette(palette::pal_t *pal, int size);
	virtual const palette::pal_t *getpalette(void) const
		{ return _palette; }
	virtual int palette_size(void) const { return _palette_size; }
	virtual palette::pal_t getpalentry(pixel_t i) const
		{ return _palette[i]; }
	virtual bool copypalette(const canvas& img);
	virtual bool copypalette(palette::pal_t *p, int size);
	uint32_t inline wcolordist(palette::pal_t *a, palette::pal_t *b);
	uint32_t inline colordist(palette::pal_t *a, palette::pal_t *b);
	virtual pixel_t lookuppalentry(palette::pal_t *p);
	virtual int lookuppalcache(palette::pal_t *p);
	virtual pixel_t findnearestpalentry(palette::pal_t *p);
	virtual bool size(int32_t width, int32_t height);
	virtual void setbg(pixel_t background){ _bgcolor = background; }
	virtual pixel_t getbg(void) { return _bgcolor; }
	virtual void rotate(int x, int y, int width, int height, int angle) {;}
	virtual void rotate(canvas& dest, int angle);
	virtual void rotate(int angle);
	virtual void scale(canvas& dest, int width, int height);
	virtual canvas scale(int width, int height);
	virtual void scale(canvas& img);
	virtual void scaleDCCI(canvas& img);

	pixel_t getbpp() const
	{
		return _bpp;
	}
};

#endif /* CANVAS_H_ */
