/*
 * vga.h
 *
 *  Created on: Jan 3, 2019
 *      Author: pedward
 */

#ifndef VGA_H_
#define VGA_H_

#ifdef __BORLANDC__
#define KBHIT kbhit
#endif

#include "image.h"
#include "adapter.h"

class vga : public adapter
{
protected:
	static const adapter::video_mode video_modes[];

protected:
	void write_crtc(unsigned int port, unsigned char reg, unsigned char val);
	bool x16mode(void);

public:
	vga(void);
	~vga(void);
	bool setup(void);
	bool setpalette(palette::pal_type pal);
	bool setpalette(palette::pal_t *pal, int palette_entries);
	bool graphmode(Mode mode);
	adapter::Mode getmode(void);
	bool textmode(void);
	void setpixel(int x, int y, unsigned char visible);
	unsigned char getpixel(int x, int y);
	void cls(void);
	ptr_t allocate_screen_buffer();
	void update(void);
	void vsync(void);
	void translate(unsigned char far *src);

};

#endif /* VGA_H_ */
