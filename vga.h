/*
 * vga.h
 *
 *  Created on: Jan 3, 2019
 *      Author: pedward
 */

#ifndef VGA_H_
#define VGA_H_

#ifndef __BORLANDC__
#include <SDL2/SDL.h>
#define SDL
#define KBHIT display.kbhit
#else
#define KBHIT kbhit
#include <dos.h>
#include <conio.h>
#endif

#include "image.h"

#define CRTCa 0x3d8
#define CRTCb 0x3d4
#define CRTCma 0x3b8
#define CRTCmb 0x3b4
#define CRTCmd 0x3ba

class vga {

public:
enum Vgamode { NONE = 0, TEXT = 0x03, VGAMONO = 0x11, VGAHI = 0x12, VGALO = 0x13, X16 = 0x15, MDA = 0x07, SDLVGALO = 0x16, SDLX16 = 0x17 };

static const unsigned char gtable[];
static const unsigned char ttable[];

typedef struct {
Vgamode mode;
unsigned int x;
unsigned int y;
unsigned int bytes;
unsigned int fp;
unsigned char planes;
unsigned char bpp;
unsigned char colors;
unsigned int sr;
unsigned int scale;
} video_mode;

static const video_mode video_modes[];

protected:
#ifdef SDL
	SDL_Window* _window;
	SDL_Surface* _screen;
	SDL_Surface* _srcscreen;
	SDL_Surface* _offscreen;
	SDL_Surface* _spritescreen;
	SDL_Renderer* _renderer;
	SDL_Texture* _texture;
	SDL_Event _event;
	unsigned int _sdlscale;
#endif
unsigned char far *_buffer;
unsigned char far *_saved_buffer;
unsigned char far *_os_buffer;
unsigned char far *_sprite_buffer;
unsigned int _width;
unsigned int _height;
unsigned int _row_bytes;
uint32_t buf_size;
unsigned char bpp;
unsigned char Bpp;
Vgamode vmode;
unsigned int SR;
enum { MONO, COLOR} card;
unsigned char sprites;
int _palette_size;
palette::pal_t *_palette;
palette::pal_type _cur_palette;

public:
unsigned char colors;
static bool SDLonce;

protected:
int bytes_per_row(void);
void write_crtc(unsigned int port, unsigned char reg, unsigned char val);
bool x16mode(void);
Vgamode getmode(void);

unsigned char planes;


public:
vga(void);
~vga(void);
bool setup(void);
void initsprites(void);
void debuginfo(void);
bool setpalette(palette::pal_type pal);
palette::pal_type getpalette(void) { return _cur_palette; }
bool sdlmode(Vgamode mode);
bool graphmode(void);
bool graphmode(Vgamode mode);
bool textmode(void);
bool mdamode(void);
void setpixel(int x, int y, unsigned char visible);
unsigned char getpixel(int x, int y);
void cls(void);
void cls(unsigned char *buf);
unsigned int width(void) { return(_width); }
unsigned int height(void) { return(_height); }
void save_buffer(void);
void restore_buffer(void);
unsigned char far *allocate_screen_buffer();
void copy_buffer(unsigned char *src);
void copyto(unsigned int x, unsigned int y, unsigned int x1, unsigned int y1, unsigned int w, unsigned int h);
void drawbox(int x,int y,int w,int h,unsigned char color);
void drawimage(unsigned int x, unsigned int y, image& img);
void drawsprite(unsigned int x, unsigned int y, image& img);
void drawsprite(unsigned int x, unsigned int y, image& img, unsigned char mask);
void syncsprites(void);
void update(void);
void vsync(void);
void writef(int col, int row, int color, char *format, ...);
void translate(unsigned char far *src);
bool detectMDA(void);
bool kbhit(void);
int getch(void);
int getEvents(int ms);


};

#endif /* VGA_H_ */
