/*
 * cga.h
 *
 *  Created on: Jan 3, 2019
 *      Author: pedward
 */

#ifndef CGA_H_
#define CGA_H_

#define PAL_REG 0x3d9
#include "image.h"
#include "adapter.h"

class cga : public adapter
{
protected:
	static const adapter::video_mode video_modes[];
	Mode _savedvmode;

	typedef union {
		unsigned char reg;
		struct {
			unsigned char border:4;
			unsigned char fg_int:1;
			unsigned char pal:1;
			unsigned char xx:2;
		} data;
	} cga_pal_t;

	cga_pal_t pal_reg;

protected:
	void write_crtc(unsigned int port, unsigned char reg, unsigned char val);
	bool x16mode(void);

public:
	cga(void);
	~cga(void);
	bool setup(void);
	bool setpalette(palette::pal_type pal);
	bool setpalette(palette::pal_t *pal, int palette_entries);
	bool graphmode(Mode mode);
	adapter::Mode getmode(void);
	bool textmode(void);
	void setmode(Mode mode);
	void overscan(unsigned char color) { pal_reg.data.border = color & 0x0F; }
	void cls(void);
	void update(void);
	void vsync(void);
	void translate(ptr_t src);

};

#endif /* CGA_H_ */
