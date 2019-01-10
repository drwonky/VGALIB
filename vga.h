/*
 * vga.h
 *
 *  Created on: Jan 3, 2019
 *      Author: pedward
 */

#ifndef VGA_H_
#define VGA_H_

#ifndef __BORLANDC__
#include <SDL/SDL.h>
#define SDL
#else
#include <dos.h>
#include <conio.h>
#endif

#include "image.h"

#define CRTCa 0x3d8
#define CRTCb 0x3d4
#define CRTCma 0x3b8
#define CRTCmb 0x3b4
#define CRTCmd 0x3ba

enum Vgamode { TEXT = 0x03, VGAMONO = 0x11, VGAHI = 0x12, VGALO = 0x13, X16 = 0x15, MDA = 0x07, SDLVGALO = 0x16 };

unsigned char gtable[]={0x35,0x2d,0x2e,0x07,
			0x5b,0x02,0x57,0x57,
			0x02,0x03,0x00,0x00};
unsigned char ttable[]={0x61,0x50,0x52,0x0f,
			0x19,0x06,0x19,0x19,
			0x02,0x0d,0x0b,0x0c};


struct video_mode {
Vgamode mode;
unsigned int x;
unsigned int y;
unsigned int bytes;
unsigned int fp;
unsigned char planes;
unsigned char bpp;
unsigned char colors;
unsigned int sr;
} video_modes[] = {
 {TEXT,80,25,80,0xb800,1,8,0xF,0x3da},
 {VGAMONO,640,480,80,0xa000,1,1,0x1,0x3da},
 {VGAHI,640,480,80,0xa000,4,1,0xF,0x3da},
 {VGALO,320,200,320,0xa000,1,8,0xFF,0x3da},
 {X16,160,100,160,0xb800,1,8,0xF,0x3da},
 {SDLVGALO,320,200,320,0xa000,1,8,0xFF,0x3da},
 {MDA,80,25,80,0xb000,1,8,0xF,0x3ba}
};

class vga {

protected:
#ifdef SDL
	SDL_Surface* screen;
	SDL_Surface* offscreen;
	SDL_Surface* spritescreen;
	SDL_Event event;
#endif
unsigned char far *buffer;
unsigned char far *saved_buffer;
unsigned char far *os_buffer;
unsigned char far *sprite_buffer;
unsigned int _width;
unsigned int _height;
unsigned int row_bytes;
uint32_t buf_size;
unsigned char bpp;
unsigned char Bpp;
Vgamode vmode;
unsigned int SR;
enum { MONO, COLOR} card;
unsigned char sprites;
int mpalette_size;
img_pal *mpalette;

public:
unsigned char colors;

private:
int bytes_per_row(void);
void write_crtc(unsigned int port, unsigned char reg, unsigned char val);
bool x16mode(void);
Vgamode getmode(void);

unsigned char planes;
static bool SDLonce;


public:
vga(void);
~vga(void);
bool setup(void);
void initsprites(void);
void debuginfo(void);
bool setpalette(pal_type pal);
bool sdlmode(void);
bool graphmode(void);
bool graphmode(Vgamode mode);
bool textmode(void);
bool mdamode(void);
void setpixel(int x, int y, unsigned char visible);
unsigned char getpixel(int x, int y);
void cls(void);
void cls(unsigned char *buf);
unsigned int width(void);
unsigned int height(void);
void save_buffer(void);
void restore_buffer(void);
unsigned char far *allocate_screen_buffer();
void copy_buffer(unsigned char *src);
void copyto(unsigned int x, unsigned int y, unsigned int x1, unsigned int y1, unsigned int w, unsigned int h);
void drawbox(int x,int y,int w,int h,unsigned char color);
void drawimage(int x, int y, image far *img);
void drawsprite(int x, int y, image& img);
void drawsprite(int x, int y, image& img, unsigned char mask);
void syncsprites(void);
void update(void);
void vsync(void);
void writef(int col, int row, int color, char *format, ...);
void translate(unsigned char far *src);
bool detectMDA(void);
bool kbhit(void);
char getch(void);


};

bool vga::SDLonce = false;


#endif /* VGA_H_ */
