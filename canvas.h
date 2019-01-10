/*
 * canvas.h
 *
 *  Created on: Jan 5, 2019
 *      Author: pedward
 */

#ifndef CANVAS_H_
#define CANVAS_H_

#include "types.h"
#include "image.h"
#include "palettes.h"

class canvas {
public:
	typedef unsigned char pixel_t;
	typedef unsigned char far * ptr_t;
	typedef char * string_t;

protected:
	int _width;
	int _height;
	pixel_t _colors;
	pixel_t _bgcolor;

	ptr_t *_buffer;
	palette::pal_t *_palette;

private:
	void initvars(void);	// c++98 does not support delegating constructors

public:
	virtual ~canvas();
	canvas();
	canvas(int width, int height);
	void setpixel(int x, int y, pixel_t color = 0);
	pixel_t getpixel(int x, int y);
	bool copybuffer(ptr_t *src);
	ptr_t getbuffer(void) { return _buffer; }
	int width(void) { return _width; }
	int height(void) { return _height; }
	virtual bool allocate(void) { return true; }
	virtual void drawbox(int x, int y, int width, int height, pixel_t color, bool filled = true);
	virtual void drawimage(int x, int y, image& img, bool transparent = false);
	virtual void drawsprite(int x, int y, image& img);
	virtual void copyto(int x1, int y1, int x2, int y2, int width, int height);
	virtual void line(int x1, int y1, int x2, int y2, pixel_t color);
	virtual void clear(pixel_t color = 0);
	virtual void drawtext(string_t str, int x, int y, pixel_t color);
	virtual void setpalette(palette::pal_type pal);
	virtual void setpalette(palette::pal_t *pal);
	virtual palette::pal_t *getpalette(void);
	virtual palette::pal_t getpalentry(pixel_t i);
	virtual int32_t wcolordist(palette::pal_t *a, palette::pal_t *b);
	virtual int32_t colordist(palette::pal_t *a, palette::pal_t *b);
	virtual pixel_t lookuppalentry(palette::pal_t *p);
	virtual pixel_t findnearestpalentry(palette::pal_t *p);
	virtual bool size(int width, int height) { _width=width; _height=height; return allocate(); }
	virtual void setbg(pixel_t background) { _bgcolor = background; }
	virtual pixel_t getbg(void) { return _bgcolor; }
	virtual void rotate(int x, int y, int width, int height, int angle);

};

#endif /* CANVAS_H_ */
