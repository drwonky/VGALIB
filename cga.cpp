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
 {CGALO1,320,200,80,0xb800,2,8,0xF,0x3da,0},
 {CGALO2,320,200,80,0xb800,2,8,0xF,0x3da,0},
 {CGAHI,640,200,80,0xb800,1,8,0xF,0x3da,0},
 {X16,160,100,160,0xb800,1,8,0xF,0x3da,4}
};

cga::cga(void)
{
	_savedvmode = getmode();
	pal_reg.reg=0;
}

cga::~cga(void)
{
	setmode(_savedvmode);
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
		case CGALO1:
			setpalette(palette::CGA1HI_PAL);
			break;
		case CGALO2:
			setpalette(palette::CGA2HI_PAL);
			break;
		case CGAHI:
			setpalette(palette::BW_PAL);
			break;
	}

	_vmode=mode;

	setmode(_vmode);
	return setup();
}

bool cga::textmode(void)
{
#ifdef __BORLANDC__
	setmode(TEXT);
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

void cga::translate(ptr_t src)
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

		xlatetext:
			asm {
				lodsw				// 5
				mov	bx, ax
				shl ax,8
				mov al,0xDB
				stosw
				mov	ax, bx
				mov al,0xDB
				stosw
				loop	xlatetext       // 15
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

		xlate16:
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
				loop	xlate16       // 15
			}                       // 44

			break;

		case CGALO1:
		case CGALO2:
			_DX=_height>>1;
			_SI=FP_OFF(src);
			_DI=FP_OFF(_buffer);
			_CX=FP_SEG(src);
			_AX=FP_SEG(_buffer);
			_BX=_row_bytes;
			_DS=_CX;
			_ES=_AX;

		xlatelo:
			asm {
				mov cx,bx
			}
		xevenlo:
			asm {
				lodsd				// 5
										//                       HHHH HHHH LLLL LLLL
				and		eax, 0x03030303	// 2 0000 00xx 0000 00xx 0000 00xx 0000 00xx
				shl		al, 2       	// 3 0000 00xx 0000 00xx 0000 00xx 0000 xx00
				or		al, ah      	// 2 0000 00xx 0000 00xx 0000 00xx 0000 xxxx
				ror		eax, 16     	// 3 0000 00xx 0000 xxxx 0000 00xx 0000 00xx
				shl		al, 2       	// 3 0000 00xx 0000 xxxx 0000 00xx 0000 xx00
				or		al, ah      	// 2 0000 00xx 0000 xxxx 0000 00xx 0000 xxxx
				shl		ax, 12			//   0000 00xx 0000 xxxx xxxx 0000 0000 0000
				shr		eax,12			//   0000 0000 0000 0000 00xx 0000 xxxx xxxx
				stosb               	// 4

				loop	xevenlo       	//

				push	di
				add		di, 0x1FB0
				mov		cx,bx
			}                       // 44
		xoddlo:
			asm {

				lodsd				// 5
										//                       HHHH HHHH LLLL LLLL
				and		eax, 0x03030303	// 2 0000 00xx 0000 00xx 0000 00xx 0000 00xx
				shl		al, 2       	// 3 0000 00xx 0000 00xx 0000 00xx 0000 xx00
				or		al, ah      	// 2 0000 00xx 0000 00xx 0000 00xx 0000 xxxx
				ror		eax, 16     	// 3 0000 00xx 0000 xxxx 0000 00xx 0000 00xx
				shl		al, 2       	// 3 0000 00xx 0000 xxxx 0000 00xx 0000 xx00
				or		al, ah      	// 2 0000 00xx 0000 xxxx 0000 00xx 0000 xxxx
				shl		ax, 12			//   0000 00xx 0000 xxxx xxxx 0000 0000 0000
				shr		eax,12			//   0000 0000 0000 0000 00xx 0000 xxxx xxxx
				stosb               	// 4

				loop	xoddlo       	//

				pop		di

				dec		dx
				jnz		xlatelo
			}                       // 44

			break;
		case CGAHI:
			_DX=_height>>2;
			_SI=FP_OFF(src);
			_DI=FP_OFF(_buffer);
			_CX=FP_SEG(src);
			_AX=FP_SEG(_buffer);
//			_BX=_row_bytes;
			_DS=_CX;
			_ES=_AX;

		xlatehi:
			asm {
				mov cx,80
			}
		xevenhi:
			asm {
				lodsd				// 5
										//                       HHHH HHHH LLLL LLLL
				shr		eax, 1
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1

				lodsd
				shr		eax, 1
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
//				mov		ax,bx
				mov		es:[di],bl
				inc		di
//				stosb               	// 4

				loop	xevenhi       	//

				push	di
				add		di, 0x1FB0
				mov		cx,80
			}                       // 44
		xoddhi:
			asm {

				lodsd				// 5
										//                       HHHH HHHH LLLL LLLL
				shr		eax, 1
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1

				lodsd
				shr		eax, 1
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
				shr		eax, 8
				rcl		bx, 1
//				mov		ax,bx
				mov		es:[di],bl
				inc		di
										//                       HHHH HHHH LLLL LLLL
				loop	xoddhi       	//

				pop		di

				dec		dx
				jnz		xlatehi
			}                       // 44

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

	switch (pal) {
	case palette::CGA0_PAL:
		pal_reg.data.pal = 0;
		pal_reg.data.fg_int = 0;
		if (_vmode != adapter::CGALO2) setmode(adapter::CGALO1);
		break;
	case palette::CGA1_PAL:
		pal_reg.data.pal = 1;
		pal_reg.data.fg_int = 0;
		if (_vmode != adapter::CGALO2) setmode(adapter::CGALO1);
		break;
	case palette::CGA0HI_PAL:
		pal_reg.data.pal = 0;
		pal_reg.data.fg_int = 1;
		if (_vmode != adapter::CGALO2) setmode(adapter::CGALO1);
		break;
	case palette::CGA1HI_PAL:
		pal_reg.data.pal = 1;
		pal_reg.data.fg_int = 1;
		if (_vmode != adapter::CGALO2) setmode(adapter::CGALO1);
		break;
	case palette::CGA2_PAL:
		pal_reg.data.pal = 0;
		pal_reg.data.fg_int = 0;
		if (_vmode != adapter::CGALO2) setmode(adapter::CGALO2);
		break;
	case palette::CGA2HI_PAL:
		pal_reg.data.pal = 0;
		pal_reg.data.fg_int = 1;
		if (_vmode != adapter::CGALO2) setmode(adapter::CGALO2);
		break;
	case palette::BW_PAL:
		pal_reg.data.pal = 0;
		pal_reg.data.fg_int = 0;
		break;
	}

	printf("pal %02x %02x\n",pal,pal_reg.reg);
//	outportb(PAL_REG,pal_reg.reg);

	return true;
}

bool cga::setpalette(palette::pal_t *pal, int palette_entries)
{
	// TODO
	return false;
}
