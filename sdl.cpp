#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "sdl.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;

/* mode, width, height, bytes_per_line, base_addr, bpp, Bpp, colors, vsync_reg, scale */
const adapter::video_mode sdl::video_modes[] = {
 {VGALO,320,200,320,0xa000,1,8,0xFF,0x3da,2},
 {VGAHI,640,480,640,0xa000,1,8,0xFF,0x3da,1},
 {X16,160,100,160,0xb800,1,8,0xF,0x3da,4}
};

bool sdl::SDLonce = false;

sdl::sdl(void)
{
	_window=NULL;
	_renderer=NULL;
	_render=NULL;
	_texture=NULL;
	_sdlscale=0;
	_screen=NULL;

	if (!SDLonce) {
		SDLonce=true;
		SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS);
	}


}

sdl::~sdl(void)
{
	if (SDLonce) {
		if (_render) SDL_FreeSurface(_render);
		if (_screen) SDL_FreeSurface(_screen);
		if (_texture) SDL_DestroyTexture(_texture);
		SDL_DestroyWindow( _window );
		SDL_Quit();
	}
}

bool sdl::setup(void)
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
			_sdlscale = video_modes[i].scale;
			_render = SDL_CreateRGBSurface(0,_width,_height,8,0,0,0,0);
			screen = canvas(_width,_height,(ptr_t)_render->pixels);
			return true;
		}
	}
	return false;
}

bool sdl::setpalette(palette::pal_type pal)

{
	adapter::setpalette(pal);

	SDL_Palette* sdl_palette = SDL_AllocPalette(_palette_size);

	for(int i=0;i<_palette_size;i++) {
		sdl_palette->colors[i].r=_palette[i].r;
		sdl_palette->colors[i].g=_palette[i].g;
		sdl_palette->colors[i].b=_palette[i].b;
	}

	SDL_SetSurfacePalette(_render,sdl_palette);
//	SDL_SetSurfacePalette(_spritescreen,sdl_palette);

	SDL_FreePalette(sdl_palette);

	return true;
}

bool sdl::setpalette(palette::pal_t *pal, int palette_entries)
{
	_palette_size=palette_entries;
	_cur_palette=palette::CUSTOM;
	_palette=new palette::pal_t[_palette_size];

	if (_palette == NULL) return false;

	memcpy(_palette,pal,sizeof(palette::pal_t)*_palette_size);

	SDL_Palette* sdl_palette = SDL_AllocPalette(_palette_size);

	for(int i=0;i<_palette_size;i++) {
		sdl_palette->colors[i].r=_palette[i].r;
		sdl_palette->colors[i].g=_palette[i].g;
		sdl_palette->colors[i].b=_palette[i].b;
	}

	SDL_SetSurfacePalette(_render,sdl_palette);
//	SDL_SetSurfacePalette(_spritescreen,sdl_palette);

	SDL_FreePalette(sdl_palette);

	return true;
}

bool sdl::sdlmode(Mode mode)
{
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
}

bool sdl::graphmode(Mode mode)
{
	switch (mode) {
		case VGAHI:
		case VGALO:
			setpalette(palette::VGA_PAL);
			return sdlmode(mode);
		case X16:
			setpalette(palette::CGA_PAL);
			return sdlmode(mode);
		default:
			return false;
	}
}

adapter::Mode sdl::getmode(void)
{
	return vmode;
}

void sdl::setpixel(int x, int y, unsigned char visible)
{
	static unsigned int plane_offset=0;

	visible&=colors;

	plane_offset=(y*_row_bytes)+(x);
	screen._buffer[plane_offset]=visible;
}

unsigned char sdl::getpixel(int x, int y)
{
	static unsigned int plane_offset=0;

	plane_offset=(y*_row_bytes)+(x);
	return(screen._buffer[plane_offset]);
}

void sdl::cls(void)
{
	screen.clear();
}

void sdl::update(void)
{
	translate(screen._buffer);
}

void sdl::translate(unsigned char far *src)
{
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
}

bool sdl::kbhit(void)
{
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
}

int sdl::getch(void)
{
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
}

/*
 * SDL routine for implementing input processing loop, since SDL needs to be called a lot of pump events
 */
int sdl::getEvents(int ms)
{
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
}

#endif
