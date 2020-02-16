/*
 * adapter.h
 *
 *  Created on: Mar 13, 2019
 *      Author: pedward
 */

#ifndef ADAPTER_H_
#define ADAPTER_H_

#include "canvas.h"
#include "types.h"

class adapter
{

public:
	enum Adapters
	{
		NONE,
		MDA,
		HERC,
		CGA,
		EGA,
		VGA,
		SDL
	};

	enum Mode
	{
		TEXT = 0x03,
		CGALO = 0x04,
		CGAHI = 0x05,
		PCJRLO = 0x08,
		PCJRMED = 0x09,
		PCJRHI = 0x0A,
		EGALO = 0x0D,
		EGAMED = 0x0E,
		EGAMONO = 0x0F,
		EGAHI = 0x10,
		VGAMONO = 0x11,
		VGAHI = 0x12,
		VGALO = 0x13,
		X16 = 0x16,
		UNDEF = 0xFF
	};

	typedef struct
	{
		int mode;
		unsigned int x;
		unsigned int y;
		unsigned int bytes;
		unsigned int fp;
		unsigned char planes;
		unsigned char bpp;
		unsigned char colors;
		unsigned int sr;
		unsigned int scale;
	} video_mode;

	canvas screen;	// backing store

protected:

	Mode vmode;
	ptr_t _buffer;	// primary screen framebuffer
	adapter *_display;
	unsigned int _row_bytes;
	unsigned int _width;
	unsigned int _height;
	uint32_t buf_size;
	unsigned char bpp;
	unsigned char Bpp;
	unsigned int SR;
	unsigned char planes;

	palette::pal_type _cur_palette;
	palette::pal_t *_palette;
	int _palette_size;

public:
	unsigned char colors;

public:
	static Adapters detect(void);
	static adapter *init(Adapters card);

	adapter(void);
	virtual ~adapter(void);
	virtual Mode getmode(void) = 0;
	virtual bool textmode(void) = 0;
	virtual bool setup(void) = 0;
	virtual ptr_t allocate_screen_buffer();
	virtual bool setpalette(palette::pal_type pal);
	virtual bool setpalette(palette::pal_t *pal, int palette_entries) = 0;
	virtual palette::pal_type getpalette(void)
	{
		return _cur_palette;
	}
	virtual const palette::pal_t *getpalette(void) const
	{
		return _palette;
	}
	virtual int width(void)
	{
		return _width;
	}
	virtual int height(void)
	{
		return _height;
	}
	virtual bool kbhit(void);
	virtual int getch(void);
	virtual bool graphmode(Mode mode) = 0;
	virtual void cls(void) = 0;
	virtual void update(void) = 0;
	virtual void translate(unsigned char far *src) = 0;
	virtual void vsync(void) = 0;

};

#endif /* ADAPTER_H_ */
