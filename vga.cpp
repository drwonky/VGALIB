#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "vga.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;
#endif

/* mode, width, height, bytes_per_line, base_addr, bpp, Bpp, colors, vsync_reg, scale */
const adapter::video_mode vga::video_modes[] = {
 {VGAMONO,640,480,80,0xa000,1,1,0x1,0x3da,0},
 {VGAHI,640,480,80,0xa000,4,1,0xF,0x3da,0},
 {VGALO,320,200,320,0xa000,1,8,0xFF,0x3da,2},
 {X16,160,100,160,0xb800,1,8,0xF,0x3da,4}
};

vga::vga(void)
{
	getmode();
}

vga::~vga(void)
{
}

ptr_t vga::allocate_screen_buffer()
{
	ptr_t buf = new unsigned far char [buf_size];
	if (buf == NULL) {
		printf ("Null ptr allocating buffer!\n");
		exit(1);
	}
	return buf;
}

bool vga::setup(void)
{
	for (unsigned long int i=0;i<sizeof(video_modes);i++) {
		if (video_modes[i].mode == vmode) {
			_row_bytes = video_modes[i].bytes;
			_width = video_modes[i].x;
			_height = video_modes[i].y;
			bpp = video_modes[i].bpp;
			Bpp = bpp/8;
			planes = video_modes[i].planes;
			colors = video_modes[i].colors;
			SR = video_modes[i].sr;
			buf_size = _width*_height;

#ifdef __BORLANDC__
			_buffer = (unsigned char far *) MK_FP(video_modes[i].fp,0);
			screen.size(_width,_height);
#endif
			return true;
		}
	}
	return false;
}

bool vga::graphmode(Mode mode)
{
	switch (mode) {
		case TEXT:
			return textmode();
		case X16:
			setpalette(palette::CGA_PAL);
			return x16mode();
		case VGALO:
			setpalette(palette::VGA_PAL);
			break;
	}

	vmode=mode;
#ifdef __BORLANDC__
	_AL=(unsigned char) mode;
	_AH=0;
	geninterrupt(0x10);
	return setup();
#else
	return true;
#endif
}

bool vga::textmode(void)
{
#ifdef __BORLANDC__
	_AL=(unsigned char) vmode;
	_AH=0;
	geninterrupt(0x10);
	return setup();
#else
	return true;
#endif
}


bool vga::x16mode(void)
{
#ifdef __BORLANDC__
	unsigned char status;

	_AL=(unsigned char) TEXT;
	_AH=0;
	geninterrupt(0x10);


	//disable VGA blink
	status=inportb(0x3da);
	outportb(0x3c0,0x30);
	status=inportb(0x3c1);

	if (status != 0xFF) {  // VGA presumed, turn 80x50 into 80x100

		status&=0xf7;
		outportb(0x3c0,status);
		write_crtc(CRTCb,0x09,0x03);

	}

	vmode=X16;

	return setup();
#else
	return true;
#endif
}

void vga::write_crtc(unsigned int port, unsigned char reg, unsigned char val)
{
#ifdef __BORLANDC__
	outportb(port, reg);
	outportb(port+1,val);
#endif
}

adapter::Mode vga::getmode(void)
{
#ifdef __BORLANDC__
	_AX=0x0f00;
	geninterrupt(0x10);
	vmode=(Mode)_AL;
	return (vmode);
#else
	return vmode;
#endif
}

void vga::setpixel(int x, int y, unsigned char visible)
{
	static unsigned int plane_offset=0;
	static unsigned char temp;
	static unsigned char bit;

	visible&=colors;

	switch (vmode) {
		case VGAMONO:
			plane_offset=(y*_row_bytes)+(x>>3);
			bit=(7-(x&7));
			temp=screen._buffer[plane_offset];
			temp=temp&(255^(1<<bit));
			temp=temp|(visible<<bit);
			screen._buffer[plane_offset]=temp;
			break;
		case VGAHI:

			break;
		default:
			plane_offset=(y*_row_bytes)+(x);
			screen._buffer[plane_offset]=visible;
			break;
	}
}

unsigned char vga::getpixel(int x, int y)
{
	static unsigned int plane_offset=0;

	switch (vmode) {
		case VGAMONO:
			plane_offset=(y*_row_bytes)+(x>>3);
			return((screen._buffer[plane_offset]&(7-(x&7)))>>(7-(x&7)));
		case VGAHI:
			break;
		case TEXT:
			plane_offset=(y*_row_bytes)+(x<<1);
			return(screen._buffer[plane_offset]);
		default:
			plane_offset=(y*_row_bytes)+(x);
			return(screen._buffer[plane_offset]);
	}
	return 0;
}

void vga::cls(void)
{
	screen.clear();
}

void vga::update(void)
{
	vsync();
	translate(screen._buffer);
}

void vga::translate(unsigned char far *src)
{
#ifdef __BORLANDC__
	switch (vmode) {
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

void vga::vsync(void)
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

bool vga::setpalette(palette::pal_type pal)
{
	// TODO
	return true;
}

bool vga::setpalette(palette::pal_t *pal, int palette_entries)
{
	// TODO
	return true;
}
