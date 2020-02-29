/*
 * ega.h
 *
 *  Created on: Jan 3, 2019
 *      Author: pedward
 */

#ifndef EGA_H_
#define EGA_H_

#define GDCi 0x3CE
#define ATCi 0x3C0
#define SCi 0x3C4

#include "image.h"
#include "adapter.h"

class ega : public adapter
{
protected:
	static const adapter::video_mode video_modes[];
	Mode _savedvmode;

protected:
	void write_crtc(unsigned int port, unsigned char reg, unsigned char val);
	bool x16mode(void);

public:
	ega(void);
	~ega(void);
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

#endif /* EGA_H_ */
