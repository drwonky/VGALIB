#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "ega.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;
#endif

/* mode, width, height, bytes_per_line, base_addr, bpp, Bpp, colors, vsync_reg, scale */
const adapter::video_mode ega::video_modes[] = {
 {TEXT,80,25,80,0xb800,1,8,0xF,0x3da,0},
 {EGAMONO,640,350,80,0xa000,1,1,0x1,0x3da,0},
 {EGAHI,640,350,320,0xa000,4,1,0xF,0x3da,0},
 {EGALO,320,200,160,0xa000,4,1,0xF,0x3da,2},
 {X16,160,100,160,0xb800,1,8,0xF,0x3da,4}
};

ega::ega(void)
{
	_savedvmode = getmode();
}

ega::~ega(void)
{
	setmode(_savedvmode);
}

bool ega::setup(void)
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
			buf_size = _row_bytes*_height;

#ifdef __BORLANDC__
			_buffer = (unsigned char far *) MK_FP(video_modes[i].fp,0);
			screen.size(_width,_height);
#endif
			return true;
		}
	}
	return false;
}

void ega::setmode(Mode mode)
{
#ifdef __BORLANDC__
	_AL=(unsigned char) mode;
	_AH=0;
	geninterrupt(0x10);
#endif
}

bool ega::graphmode(Mode mode)
{
	switch (mode) {
		case TEXT:
			return textmode();
		case X16:
			setpalette(palette::TEXT_PAL);
			return x16mode();
		case EGALO:
		case EGAHI:
			setpalette(palette::TEXT_PAL);
			break;
		case EGAMONO:
			setpalette(palette::BW_PAL);
			break;
		case CGALO1:
		case CGALO2:
		case CGAHI:
		case COMPAQ:
		case HERC:
		case BWTEXT:
		case PCJRLO:
		case PCJRMED:
		case PCJRHI:
		case EGAMED:
		case VGAMONO:
		case VGAHI:
		case VGALO:
		case SVGA:
		case XGA:
		case HD:
		case FHD:
		case UNDEF:
			return false;
	}

	_vmode=mode;

	setmode(_vmode);
	return setup();
}

bool ega::textmode(void)
{
#ifdef __BORLANDC__
	_AL=0x03;
	_AH=0;
	geninterrupt(0x10);
	return setup();
#else
	return true;
#endif
}


bool ega::x16mode(void)
{
#ifdef __BORLANDC__
	unsigned char status;
	unsigned char switches;

	_AL=(unsigned char) TEXT;
	_AH=0;
	geninterrupt(0x10);

	_AH=0x12;
	_AL=0;
	_BL=0x10;
	geninterrupt(0x10);		/* get EGA switch settings */

	switches = _CL;

	_AL=0x03;
	_AH=0x10;
	_BL=0;
	geninterrupt(0x10);		/* turn off blink via EGA BIOS */

	/* EGA hires mode is 640x350 with a 9x14 character cell.  The pixel aspect
		ratio is 1:1.37, so if we make the blocks 3 scans tall you get a square
		pixel at 160x100, but some of the scan lines are not used (50) */

	if (
		switches == 0x09 ||		/* EGA Hires monitor attached, 9x14 */
		switches == 0xB		/* EGA with Monochrome monitor, 9x14 */
		) {
			write_crtc(CRTCb,0x09,0x02);
		} else {						/* Must be CGA 8x8 on EGA card */
			write_crtc(CRTCb,0x09,0x01);
		}


	_vmode=X16;

	return setup();
#else
	return true;
#endif
}

void ega::write_crtc(unsigned int port, unsigned char reg, unsigned char val)
{
#ifdef __BORLANDC__
	outportb(port, reg);
	outportb(port+1,val);
#endif
}

adapter::Mode ega::getmode(void)
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

void ega::cls(void)
{
	screen.clear();
	update();
}

void ega::update(void)
{
	vsync();
	translate(screen._buffer);
}

void ega::translate(ptr_t src)
{
#ifdef __BORLANDC__
	switch (_vmode) {
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
#ifdef i86
			_CX=buf_size>>1;
			_DI=FP_OFF(_buffer);
			_SI=FP_OFF(src);
			_BX=FP_SEG(src);
			_DS=_BX;
			_AX=FP_SEG(_buffer);
			_ES=_AX;

		xlate16:
			asm {
				lodsw				// 5
				shl		al, 4       // 3
				or		ah, al      // 2
				mov		al, 0xde    // 2
				stosw               // 4
				loop	xlate16       // 15
			}                       // 44
#else
			_CX=buf_size>>2;
			_DI=FP_OFF(_buffer);
			_SI=FP_OFF(src);
			_BX=FP_SEG(src);
			_DS=_BX;
			_AX=FP_SEG(_buffer);
			_ES=_AX;

		xlate32:
			asm {
				lodsd				// 5
				//and		eax, 0x0F0F0F0F // 2
				shl		al, 4       // 3
				or		ah, al      // 2
				mov		al, 0xde    // 2
				ror		eax, 16     // 3
				shl		al, 4       // 3
				or		ah, al      // 2
				mov		al, 0xde    // 2
				rol		eax, 16     // 3
				stosd               // 4
				loop	xlate32       // 15
			}                       // 44
#endif

			break;

		case EGALO:
			_CX=buf_size<<1;
			_DI=FP_OFF(_buffer);
			_SI=FP_OFF(src);
			_BX=FP_SEG(src);
			_DS=_BX;
			_AX=FP_SEG(_buffer);
			_ES=_AX;

			asm {					// setup write mode 2
				mov		dx, GDCi
				mov		al, 5
				out		dx, al
				inc		dx
				mov		al, 0x02
				out		dx, al
				mov		dx, SCi
				mov		al, 2
				out		dx, al
				mov		al, 0x0F
				inc		dx
				out		dx, al
				mov		dx, GDCi
				mov		al, 3
				out 	dx, al
				inc		dx
				mov		al, 0
				out		dx, al
				mov		dx, GDCi
				mov		al, 8
				out		dx, al		// select bit mask register
				inc		dx
				mov		al, 0x80
			}
		xlatelo:
			asm{
				out		dx, al		// mask current pixel bit
				mov		ah,al
				mov		al,es:[di]
				lodsb
				stosb
				mov		al,ah
				ror		al, 1		// shift mask right 1 bit/pixel
				jc		incdi
				dec		di
			}
			incdi:
			asm {
				loop	xlatelo
			}                       // 44

			break;
	}
#endif
}

void ega::vsync(void)
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

bool ega::setpalette(palette::pal_type pal)
{
	// TODO
	_cur_palette = pal;
	return true;
}

bool ega::setpalette(palette::pal_t *pal, int palette_entries)
{
	// TODO
	return true;
}
