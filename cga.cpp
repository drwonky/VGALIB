#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "cga.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;
#endif

/* mode, width, height, bytes_per_line, base_addr, bpp, Bpp, colors, vsync_reg, scale */
const adapter::video_mode cga::video_modes[] = {
 {TEXT,80,25,80,0xb800,8,8,0xF,0x3da,0},
 {CGALO,320,200,80,0xb800,2,8,0xF,0x3da,0},
 {CGAHI,640,200,80,0xb800,1,8,0xF,0x3da,0},
 {X16,160,100,160,0xb800,1,8,0xF,0x3da,4}
};

cga::cga(void)
{
	_savedvmode = getmode();
}

cga::~cga(void)
{
	setmode(_savedvmode);
}

ptr_t cga::allocate_screen_buffer()
{
	ptr_t buf = new unsigned far char [buf_size];
	if (buf == NULL) {
		printf ("Null ptr allocating buffer!\n");
		exit(1);
	}
	return buf;
}

bool cga::setup(void)
{
	for (unsigned long int i=0;i<sizeof(video_modes);i++) {
		if (video_modes[i].mode == _vmode) {
			_row_bytes = video_modes[i].bytes;
			_width = video_modes[i].x;
			_height = video_modes[i].y;
			bpp = video_modes[i].bpp;
			Bpp = 8/bpp;
			planes = video_modes[i].planes;
			colors = video_modes[i].colors;
			SR = video_modes[i].sr;
			buf_size = _width*_height/Bpp;

#ifdef __BORLANDC__
			_buffer = (unsigned char far *) MK_FP(video_modes[i].fp,0);
			screen.size(_width,_height);
#endif
			return true;
		}
	}
	return false;
}

void cga::setmode(Mode mode)
{
#ifdef __BORLANDC__
	_AL=(unsigned char) mode;
	_AH=0;
	geninterrupt(0x10);
#endif
}

bool cga::graphmode(Mode mode)
{
	switch (mode) {
		case TEXT:
			return textmode();
		case X16:
			setpalette(palette::TEXT_PAL);
			return x16mode();
		case CGALO:
			setpalette(palette::CGA1_PAL);
			break;
	}

	_vmode=mode;

	setmode(_vmode);
	return setup();
}

bool cga::textmode(void)
{
#ifdef __BORLANDC__
	setmode(3);
	return setup();
#else
	return true;
#endif
}


bool cga::x16mode(void)
{
#ifdef __BORLANDC__
	unsigned char status;

	setmode(TEXT);

    /* set mode control register for 80x25 text mode and disable video output */
    outportb(CRTCa, 1);

    /*
            These settings put the 6845 into "graphics" mode without actually
            switching the CGA controller into graphics mode.  The register
            values are directly copied from CGA graphics mode register
            settings.  The 6845 does not directly display graphics, the
            6845 only generates addresses and sync signals, the CGA
            attribute controller either displays character ROM data or color
            pixel data, this is external to the 6845 and keeps the CGA card
            in text mode.
            ref: HELPPC
    */

    /* set vert total lines to 127 */
    write_crtc(CRTCb,0x04,0x7f);
    /* set vert displayed char rows to 100 */
    write_crtc(CRTCb,0x06,0x64);
    /* set vert sync position to 112 */
    write_crtc(CRTCb,0x07,0x70);
    /* set char scan line count to 1 */
    write_crtc(CRTCb,0x09,0x01);

    /* re-enable the video output in 80x25 text mode */
    outportb(CRTCa, 9);

	_vmode=X16;

	return setup();
#else
	return true;
#endif
}

void cga::write_crtc(unsigned int port, unsigned char reg, unsigned char val)
{
#ifdef __BORLANDC__
	outportb(port, reg);
	outportb(port+1,val);
#endif
}

adapter::Mode cga::getmode(void)
{
#ifdef __BORLANDC__
	_AX=0x0f00;
	geninterrupt(0x10);
	_vmode=(Mode)_AL;
	return (_vmode);
#else
	return _vmode;
#endif
}

void cga::cls(void)
{
	screen.clear();
	update();
}

void cga::update(void)
{
	vsync();
	translate(screen._buffer);
}

void cga::translate(unsigned char far *src)
{
#ifdef __BORLANDC__
	switch (_vmode) {
		case MDA:
		case TEXT:
			_CX=buf_size>>1;
			_DI=FP_OFF(_buffer);
			_SI=FP_OFF(src);
			_BX=FP_SEG(src);
			_DS=_BX;
			_AX=FP_SEG(_buffer);
			_ES=_AX;

		xlate:
			asm {
				lodsw				// 5
				mov	bx, ax
				shl ax,8
				mov al,0xDB
				stosw
				mov	ax, bx
				mov al,0xDB
				stosw
				loop	xlate       // 15
			}                       // 44

			break;
		case X16:
			_CX=buf_size>>2;
			_DI=FP_OFF(_buffer);
			_SI=FP_OFF(src);
			_BX=FP_SEG(src);
			_DS=_BX;
			_AX=FP_SEG(_buffer);
			_ES=_AX;

		xlate1:
			asm {
				lodsd				// 5
				and		eax, 0x0F0F0F0F // 2
				shl		al, 4       // 3
				or		ah, al      // 2
				mov		al, 0xde    // 2
				ror		eax, 16     // 3
				shl		al, 4       // 3
				or		ah, al      // 2
				mov		al, 0xde    // 2
				rol		eax, 16     // 3
				stosd               // 4
				loop	xlate1       // 15
			}                       // 44

			break;

		case VGALO:
			memory::blit(_buffer,src,buf_size);
			break;
	}
#endif
}

void cga::vsync(void)
{
#ifdef __BORLANDC__
	unsigned char p;

		do {
			p=inportb(SR);
		} while (p & 8);

		do {
			p=inportb(SR);
		} while (!(p & 8));
#endif
}

bool cga::setpalette(palette::pal_type pal)
{
	// TODO
	_cur_palette = pal;
	return true;
}

bool cga::setpalette(palette::pal_t *pal, int palette_entries)
{
	// TODO
	return true;
}
