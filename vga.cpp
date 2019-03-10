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

const unsigned char vga::gtable[]={0x35,0x2d,0x2e,0x07,
			0x5b,0x02,0x57,0x57,
			0x02,0x03,0x00,0x00};
const unsigned char vga::ttable[]={0x61,0x50,0x52,0x0f,
			0x19,0x06,0x19,0x19,
			0x02,0x0d,0x0b,0x0c};

/* mode, width, height, bytes_per_line, base_addr, bpp, Bpp, colors, vsync_reg, scale */
const vga::video_mode vga::video_modes[] = {
 {TEXT,80,25,80,0xb800,1,8,0xF,0x3da,0},
 {VGAMONO,640,480,80,0xa000,1,1,0x1,0x3da,0},
 {VGAHI,640,480,80,0xa000,4,1,0xF,0x3da,0},
 {VGALO,320,200,320,0xa000,1,8,0xFF,0x3da,2},
 {X16,160,100,160,0xb800,1,8,0xF,0x3da,4},
 {SDLVGALO,320,200,320,0xa000,1,8,0xFF,0x3da,2},
 {SDLX16,160,100,160,0xb800,1,8,0xF,0x3da,4},
 {MDA,80,25,80,0xb000,1,8,0xF,0x3ba,0}
};

bool vga::SDLonce = false;

vga::vga(void)
{
#ifdef SDL
	_window=NULL;
	_renderer=NULL;
	_render=NULL;
	_texture=NULL;
	_sdlscale=0;
	_screen=NULL;
#endif

	colors=0;
	_width=0;
	_height=0;
	_palette=NULL;
	_palette_size=0;
	_cur_palette=palette::NONE;
	_row_bytes=0;
	bpp=0;
	Bpp=0;
	SR=0;
	planes=0;
	_buffer=NULL;
	buf_size=0;

#ifndef SDL
	getmode();

	if (vmode == MDA) {
		card = MONO;
	} else {
		card = COLOR;
	}
#else
	vmode = NONE;

	if (!SDLonce) {
		SDLonce=true;
		SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS);
	}

	card=COLOR;
#endif

}

vga::~vga(void)
{
#ifdef SDL
	if (SDLonce) {
		if (_render) SDL_FreeSurface(_render);
		if (_screen) SDL_FreeSurface(_screen);
		if (_texture) SDL_DestroyTexture(_texture);
		SDL_DestroyWindow( _window );
		SDL_Quit();
	}
#endif
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
#ifndef SDL
			_buffer = (unsigned char far *) MK_FP(video_modes[i].fp,0);
			screen.size(_width,_height);
#else
			_sdlscale = video_modes[i].scale;
			_render = SDL_CreateRGBSurface(0,_width,_height,8,0,0,0,0);
			screen = canvas(_width,_height,(ptr_t)_render->pixels);
#endif
			return true;
		}
	}
	return false;
}

bool vga::setpalette(palette::pal_type pal)
{
	_palette_size=palette::palettes[pal].palette_entries;
	_cur_palette=pal;
	_palette=new palette::pal_t[_palette_size];

	if (_palette == NULL) return false;

	memcpy(_palette,palette::palettes[pal].pal,sizeof(palette::pal_t)*_palette_size);

#ifdef SDL
	SDL_Palette* sdl_palette = SDL_AllocPalette(_palette_size);

	for(int i=0;i<_palette_size;i++) {
		sdl_palette->colors[i].r=_palette[i].r;
		sdl_palette->colors[i].g=_palette[i].g;
		sdl_palette->colors[i].b=_palette[i].b;
	}

	SDL_SetSurfacePalette(_render,sdl_palette);
//	SDL_SetSurfacePalette(_spritescreen,sdl_palette);

	SDL_FreePalette(sdl_palette);
#endif

	return true;
}

bool vga::graphmode(void)
{
#ifndef SDL
	vmode = VGAMONO;
	_AL=(unsigned char) VGAMONO;
	_AH=0;
	geninterrupt(0x10);
	return setup();
#else
	return true;
#endif
}

bool vga::sdlmode(Vgamode mode)
{
#ifdef SDL
	vmode=mode;
	setup();

	_window = SDL_CreateWindow("VGALIB",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,_width*_sdlscale,_height*_sdlscale,SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "1", SDL_HINT_OVERRIDE);
	SDL_RenderClear(_renderer);
	SDL_RenderSetLogicalSize(_renderer, _width, _height);
	_texture = SDL_CreateTexture(_renderer,
			SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            _width, _height);

	SDL_SetRenderTarget(_renderer, _texture);

	_screen = SDL_CreateRGBSurface(0, _width, _height, 32, 0, 0, 0, 0);

	return false;
#else
	mode=mode; // silence annoying compile warning
	return true;
#endif
}

bool vga::graphmode(Vgamode mode)
{
#ifdef SDL
	switch (mode) {
		case VGALO:
		case SDLVGALO:
			setpalette(palette::VGA_PAL);
			return sdlmode(mode);
		case X16:
		case SDLX16:
			setpalette(palette::CGA_PAL);
			return sdlmode(mode);
		default:
			return false;
	}
#else
	switch (mode) {
		case MDA:
			return mdamode();
		case TEXT:
			return textmode();
		case X16:
			setpalette(palette::CGA_PAL);
			if (card == MONO) return mdamode();
			return x16mode();
		case VGALO:
			setpalette(palette::VGA_PAL);
			break;
	}

	vmode=mode;
	_AL=(unsigned char) mode;
	_AH=0;
	geninterrupt(0x10);
	return setup();
#endif
}

bool vga::textmode(void)
{
	if (card == MONO) {
		vmode = MDA;
	} else {
		vmode = TEXT;
	}

#ifndef SDL
	_AL=(unsigned char) vmode;
	_AH=0;
	geninterrupt(0x10);
	return setup();
#else
	return true;
#endif
}

bool vga::detectMDA(void)
{
#ifndef SDL
	unsigned int i;

	for (i=0;i<32768;i++) {
		if ((inportb(CRTCmd) & 0x80) == 0) break;
	}

	return i == 32768 ? false : true;
#else
	return false;
#endif
}

bool vga::mdamode(void)
{
#ifndef SDL
	_AL=(unsigned char) MDA;
	_AH=0;
	geninterrupt(0x10);

	outportb(CRTCma, 0);  // disable screen

	write_crtc(CRTCmb,0x0a,0x10);  // turn off cursor

	outportb(CRTCma, 8);  // enable screen, disable blink

	vmode=MDA;

	return setup();
#else
	return false;
#endif
}

bool vga::x16mode(void)
{
#ifndef SDL
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

	} else { // CGA assumed

		outportb(CRTCa, 1);

		write_crtc(CRTCb,0x04,0x7f);
		write_crtc(CRTCb,0x06,0x64);
		write_crtc(CRTCb,0x07,0x70);
		write_crtc(CRTCb,0x09,0x01);

		outportb(CRTCa, 9);
	}

	vmode=X16;

	return setup();
#else
	return true;
#endif
}

void vga::write_crtc(unsigned int port, unsigned char reg, unsigned char val)
{
#ifndef SDL
	outportb(port, reg);
	outportb(port+1,val);
#endif
}

vga::Vgamode vga::getmode(void)
{
#ifndef SDL
	_AX=0x0f00;
	geninterrupt(0x10);
	vmode=(Vgamode)_AL;
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
#ifndef SDL
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
#else
	void *pixels;
	int pitch;

	SDL_BlitSurface(_render, NULL, _screen, NULL);

	SDL_LockTexture(_texture, NULL, &pixels, &pitch);

	SDL_ConvertPixels(_screen->w, _screen->h, _screen->format->format,
			_screen->pixels, _screen->pitch, SDL_PIXELFORMAT_ARGB8888, pixels,
			pitch);
	SDL_UnlockTexture(_texture);

	SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	SDL_RenderPresent(_renderer);
#endif
}

void vga::vsync(void)
{
#ifndef SDL
	unsigned char p;

	if (card == COLOR) {
		do {
			p=inportb(SR);
		} while (p & 8);

		do {
			p=inportb(SR);
		} while (!(p & 8));
	} else {
		do {
			p=inportb(SR);
		} while (p & 0x80);

		do {
			p=inportb(SR);
		} while (!(p & 0x80));
	}
#endif
}

void vga::writef(int col, int row, int color, char *format, ...)
/* Prints a string in text video memory at a selected location in a color */
{
	va_list arg_ptr;
	char output[81];
	int len,start,i;

	va_start(arg_ptr, format);
	len = vsprintf(output, format, arg_ptr);
	output[len] = 0;
	start=row*160+(col<<1);
	i=0;
	while(i<len-1) {
		_buffer[start]=output[i];
		_buffer[start+1]=color&0x0F;
		start+=2;
		i++;
	}
} /* writef */

bool vga::kbhit(void)
{
#ifdef SDL
	if (SDL_PollEvent(&_event)) {
		switch (_event.type) {
		case SDL_QUIT:
		case SDL_KEYDOWN:
			std::cout<<"caught event "<<std::hex<<_event.type<<" keysym "<<_event.key.keysym.sym<<"("<<std::dec<<_event.key.keysym.sym<<")"<<std::endl;
			return true;
			break;
		}
	}
	return false;
#else
	return kbhit();
#endif
}

int vga::getch(void)
{
#ifdef SDL
	while (SDL_WaitEvent(&_event)) {
		/* an event was found */
		switch (_event.type) {
		/* close button clicked */
		case SDL_QUIT:
			return 0;
			break;

			/* handle the keyboard */
		case SDL_KEYDOWN:
			return _event.key.keysym.sym;
			break;
		}
	}

	return 0;
#else
	return getchar();
#endif
}

/*
 * SDL routine for implementing input processing loop, since SDL needs to be called a lot of pump events
 */
int vga::getEvents(int ms)
{
#ifdef SDL
	uint32_t start_time, sched_time;

	start_time=SDL_GetTicks();
	sched_time=start_time+ms;

	do {
		if (SDL_PollEvent(&_event)) {
//		SDL_WaitEventTimeout(&_event,ms);
		/* an event was found */
		switch (_event.type) {
		/* close button clicked */
		case SDL_QUIT:
			return -1;
			break;

			/* handle the keyboard */
		case SDL_KEYDOWN:
			return _event.key.keysym.sym;
			break;
		default:
//			return 0;
			break;
			}
		}
	} while (SDL_GetTicks() < sched_time);

	return 0;
#else
	delay(ms);
	return this->kbhit();
#endif
}
