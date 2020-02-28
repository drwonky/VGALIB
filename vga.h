/*
 * vga.h
 *
 *  Created on: Jan 3, 2019
 *      Author: pedward
 */

#ifndef VGA_H_
#define VGA_H_

#include "image.h"
#include "adapter.h"

class vga : public adapter
{
protected:
	static const adapter::video_mode video_modes[];
	Mode _savedvmode;

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
	void setmode(Mode mode);
	void cls(void);
	void update(void);
	void vsync(void);
	void translate(ptr_t src);

};

#endif /* VGA_H_ */
