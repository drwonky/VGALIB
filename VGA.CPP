#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "png.h"

#define CRTCa 0x3d8
#define CRTCb 0x3d4
#define CRTCma 0x3b8
#define CRTCmb 0x3b4
#define CRTCmd 0x3ba

enum Vgamode { TEXT = 0x03, VGAMONO = 0x11, VGAHI = 0x12, VGALO = 0x13, X16 = 0x15, MDA = 0x07 };

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
 {MDA,80,25,80,0xb000,1,8,0xF,0x3ba}
};

class vga {

protected:
unsigned char far *buffer;
unsigned char far *saved_buffer;
unsigned char far *os_buffer;
unsigned char far *sprite_buffer;
unsigned int *tsvscrn;
unsigned int x_w;
unsigned int y_h;
unsigned int row_bytes;
unsigned long buf_size;
unsigned char temp;
unsigned char bit;
unsigned char bpp;
unsigned char Bpp;
Vgamode vmode;
unsigned int SR;
enum { MONO, COLOR} card;
unsigned char sprites;

public:
unsigned char colors;

private:
int bytes_per_row(void);
void write_crtc(unsigned int port, unsigned char reg, unsigned char val);
bool x16mode(void);

unsigned char planes;

protected:
virtual Vgamode getmode(void);

public:
vga(void);
~vga(void);
bool setup(void);
void initsprites(void);
void debug(void);
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
unsigned char far *vga::allocate_screen_buffer();
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

};

vga::vga(void)
{
	unsigned char mode;
	saved_buffer = new unsigned far char[1];
	os_buffer = new unsigned far char[1];
	sprite_buffer = new unsigned far char[1];
	sprites=0;

	getmode();

	if (vmode == MDA) {
		card = MONO;
	} else {
		card = COLOR;
	}

	setup();

}

vga::~vga(void)
{
	printf("vga destructor called\n");
	delete[] saved_buffer;
	delete[] os_buffer;
	delete[] sprite_buffer;
}

void vga::debug(void)
{
	printf("vmode: %d\n", vmode);
	printf("buffer: %Fp\n", buffer);
	printf("os buffer: %Fp\n", os_buffer);
	printf("sprite buffer: %Fp\n", sprite_buffer);
	printf("x_w: %d\n",x_w);
	printf("y_h: %d\n",y_h);
	printf("row_bytes: %d\n",row_bytes);
	printf("buf_size: %lu\n",buf_size);

}

unsigned char far *vga::allocate_screen_buffer()
{
	unsigned char far *buf = new unsigned far char [buf_size];
	if (buf == NULL) {
		printf ("Null ptr allocating buffer!\n");
		exit(1);
	}
	return buf;
}

bool vga::setup(void)
{
	int i;

	delete[] os_buffer;
	for (i=0;i<sizeof(video_modes);i++) {
		if (video_modes[i].mode == vmode) {
			row_bytes = video_modes[i].bytes;
			x_w = video_modes[i].x;
			y_h = video_modes[i].y;
			bpp = video_modes[i].bpp;
			Bpp = bpp/8;
			planes = video_modes[i].planes;
			colors = video_modes[i].colors;
			SR = video_modes[i].sr;
			buffer = (unsigned char far *) MK_FP(video_modes[i].fp,0);
			buf_size = x_w*y_h;
			os_buffer = allocate_screen_buffer();
			return true;
		}
	}
	return false;
}

void vga::initsprites(void)
{
	sprite_buffer=allocate_screen_buffer();
	cls(sprite_buffer);
	sprites=1;
}

bool vga::graphmode(void)
{
	vmode = VGAMONO;
	_AL=(unsigned char) VGAMONO;
	_AH=0;
	geninterrupt(0x10);
	return setup();
}

bool vga::graphmode(Vgamode mode)
{
	switch (mode) {
		case MDA:
			return mdamode();
		case TEXT:
			return textmode();
		case X16:
			if (card == MONO) return mdamode();
			return x16mode();
	}

	vmode=mode;
	_AL=(unsigned char) mode;
	_AH=0;
	geninterrupt(0x10);
	return setup();
}

bool vga::textmode(void)
{
	if (card == MONO) {
		vmode = MDA;
	} else {
		vmode = TEXT;
	}
	_AL=(unsigned char) vmode;
	_AH=0;
	geninterrupt(0x10);
	return setup();
}

bool vga::detectMDA(void)
{
	unsigned int i;

	for (i=0;i<32768;i++) {
		if ((inportb(CRTCmd) & 0x80) == 0) break;
	}

	return i == 32768 ? false : true;
}

bool vga::mdamode(void)
{
	_AL=(unsigned char) MDA;
	_AH=0;
	geninterrupt(0x10);

	outportb(CRTCma, 0);  // disable screen

	write_crtc(CRTCmb,0x0a,0x10);  // turn off cursor

	outportb(CRTCma, 8);  // enable screen, disable blink

	vmode=MDA;

	return setup();
}

bool vga::x16mode(void)
{
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
}

void vga::write_crtc(unsigned int port, unsigned char reg, unsigned char val)
{
	outportb(port, reg);
	outportb(port+1,val);
}

Vgamode vga::getmode(void)
{
	_AX=0x0f00;
	geninterrupt(0x10);
	vmode=(Vgamode)_AL;
	return (vmode);
}

void vga::setpixel(int x, int y, unsigned char visible)
{
	static unsigned int plane_offset=0;
	static unsigned char temp;
	static unsigned char bit;

	visible&=colors;

	switch (vmode) {
		case VGAMONO:
			plane_offset=(y*row_bytes)+(x>>3);
			bit=(7-x&7);
			temp=os_buffer[plane_offset];
			temp=temp&(255^(1<<bit));
			temp=temp|(visible<<bit);
			os_buffer[plane_offset]=temp;
			break;
		case VGAHI:

			break;
		case TEXT:
		case VGALO:
		case X16:
		case MDA:
			plane_offset=(y*row_bytes)+(x);
			os_buffer[plane_offset]=visible;
			break;
		case 99:
			plane_offset=(y*row_bytes)+(x&0xFE)+1;
			if (x&1) {
				visible|=os_buffer[plane_offset]&0xF0;
			} else {
				visible<<=4;
				visible|=os_buffer[plane_offset]&0x0F;
			}
			os_buffer[plane_offset]=visible;
			break;

	}
}

unsigned char vga::getpixel(int x, int y)
{
	static unsigned int plane_offset=0;
	static unsigned char temp;

	switch (vmode) {
		case VGAMONO:
			plane_offset=(y*row_bytes)+(x>>3);
			return((os_buffer[plane_offset]&(7-(x&7)))>>(7-(x&7)));
		case VGAHI:
			temp=0;
			break;
		case TEXT:
			plane_offset=(y*row_bytes)+(x<<1);
			return(os_buffer[plane_offset]);
		case VGALO:
			plane_offset=(y*row_bytes)+(x);
			return(os_buffer[plane_offset]);
	}
	return 0;
}

void vga::cls(void)
{
	cls(os_buffer);
}

void vga::cls(unsigned char *buf)
{
	unsigned char far *p;
	switch (vmode) {
		case X16:
		case TEXT:
		case MDA:
		case VGALO:
		case VGAMONO:
			memset((void *)buf,0,buf_size);
			break;
		case VGAHI:

			break;

	}
}

unsigned int vga::width(void)
{
	return(x_w);
}

unsigned int vga::height(void)
{
	return(y_h);
}

void vga::save_buffer(void)
{
	delete saved_buffer;

	saved_buffer = allocate_screen_buffer();
	switch (vmode) {
		case VGALO:
		case VGAMONO:
		case TEXT:
			_fmemcpy(saved_buffer,&buffer,row_bytes*y_h*planes);
			break;
		case VGAHI:
			_fmemcpy(saved_buffer,&buffer,row_bytes*y_h*planes);
			break;
	}
}

void vga::restore_buffer(void)
{
	copy_buffer(saved_buffer);
}

void vga::syncsprites(void)
{
	memory::blit(sprite_buffer,os_buffer,buf_size);
}

void vga::update(void)
{
	if (sprites) {
		vsync();
		translate(sprite_buffer);
	} else {
		vsync();
		translate(os_buffer);
	}
}

void vga::copy_buffer(unsigned char far *src)
{
	switch (vmode) {
		case VGAHI:
			memory::fast_memcpy(buffer,src,buf_size); // TODO: calc and plane switch
			break;
		default:
			memory::fast_memcpy(buffer,src,buf_size);
			break;
	}
}

void vga::copyto(unsigned int x, unsigned int y, unsigned int x1, unsigned int y1, unsigned int w, unsigned int h)
{
	static unsigned int src,rem,dest,cnt;
	unsigned char xtmp,sodd;
	unsigned char far *tmp;

	if (x1>x_w-1 || y1>y_h) return;

	w=x1+w > x_w ? x_w-x1 : w;
	h=y1+h > y_h ? y_h-y1 : h;
	switch (vmode) {
		case X16:
		case MDA:
		case VGALO:
		case TEXT:
			src=y*row_bytes+(x*Bpp);
			dest=y1*row_bytes+(x1*Bpp);
			cnt=h;
			tmp=new unsigned char[w*h*Bpp];
			rem=0;
			while(cnt) {
				memory::fast_memcpy(&tmp[rem],&os_buffer[src],w*Bpp);
				rem+=w*Bpp;
				src+=row_bytes;
				cnt--;
			}
			cnt=h;
			rem=0;
			while(cnt) {
				memory::fast_memcpy(&os_buffer[dest],&tmp[rem],w*Bpp);
				dest+=row_bytes;
				rem+=w*Bpp;
				cnt--;
			}
			delete[] tmp;
			break;
		case VGAMONO:
			break;
		case VGAHI:
			//_fmemcpy(buffer,src,buf_size); // TODO: calc and plane switch
			break;
	}
}

void vga::translate(unsigned char far *src)
{
	switch (vmode) {
		case MDA:
		case TEXT:
			_CX=buf_size>>1;
			_DI=FP_OFF(buffer);
			_SI=FP_OFF(src);
			_BX=FP_SEG(src);
			_DS=_BX;
			_AX=FP_SEG(buffer);
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
			_DI=FP_OFF(buffer);
			_SI=FP_OFF(src);
			_BX=FP_SEG(src);
			_DS=_BX;
			_AX=FP_SEG(buffer);
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
			memory::blit(buffer,src,buf_size);
			break;
	}
}


void vga::drawbox(int x,int y,int w,int h,unsigned char color)
{
	int x1,y1;
	for(y1=y;y1<y+h;y1++)
	{
		for(x1=x;x1<x+w;x1++)
		{
			 setpixel(x1,y1,color);
		}
	}
}

void vga::drawimage(int x, int y, image far *img)
{
	unsigned int rem,dest,cnt,w,h,iw;

	iw=w=img->width();
	h=img->height();

	if (x>x_w-1 || y>y_h-1 || x<0 || y<0) return;

	w=x+w > x_w-1 ? x_w-x : w;
	h=y+h > y_h-1 ? y_h-y : h;
	dest=y*row_bytes+x;
	cnt=h;
	rem=0;
	while(cnt) {
		memory::fast_memcpy(&os_buffer[dest],&img->buffer[rem],w);
		dest+=row_bytes;
		rem+=iw;
		cnt--;
	}
}

void vga::drawsprite(int x, int y, image& img)
{
	drawsprite(x,y,img,img.getbg());
}

void vga::drawsprite(int x, int y, image& img, unsigned char mask)
{
	unsigned int cnt,w,h,iw;
	unsigned char far *s,*d;

	iw=w=img.width();
	h=img.height();

	if (x>x_w-1 || y>y_h-1 || x<0 || y<0) return;

	w=x+w > x_w-1 ? x_w-x : w;
	h=y+h > y_h-1 ? y_h-y : h;
	cnt=h;
	d=&sprite_buffer[y*row_bytes+x];
	s=img.buffer;
	while(cnt) {
		memory::mask_memcpy(d,s,w,mask);
		d+=row_bytes;
		s+=iw;
		cnt--;
	}
}

void vga::vsync(void)
{
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
}

void vga::writef(int col, int row, int color, char *format, ...)
/* Prints a string in video memory at a selected location in a color */
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
		buffer[start]=output[i];
		buffer[start+1]=color&0x0F;
		start+=2;
		i++;
	}
} /* writef */

void main(void)
{
	int x,y,w,h,dx,dy;
	unsigned long i;
	unsigned char color=1;
	unsigned char c=0;
	unsigned int rot=1;
	vga display;
	png mariopng;
	image mario;

	w=20;
	h=20;
	dx=1;
	dy=1;
	x=0;
	y=0;

	mariopng.load("mario16.png");

	mariopng.convert2image(mario);

	w=mario.width();
	h=mario.height();

	image box(w,h);
	image boxc(w,h);
	image boxd(w*2,h*2);
	image boxe(w/2,h/2);

	box.drawbox(5,5,w-11,h-11,color);
	box.drawbox(6,6,w-13,h-13,color+8);
	box.drawbox(8,8,w-17,h-17,0);

	display.graphmode(VGALO);
	display.initsprites();
	display.cls();
	getch();
//	display.debug();

	for(x=0;x<display.width();x++) {
		for(y=0;y<display.height();y++) {
			display.setpixel(x,y,(x/(display.width()/display.colors)));
//			display.setpixel(x,y,7);
		}
	}

	display.syncsprites();
	display.update();

	x=y=0;
//	display.drawbox(x,y,w,h,color+1);
//	display.drawbox(x+3,y+3,w-6,h-6,color+2);
	display.syncsprites();
	//display.drawsprite(x,y,box);
	display.drawsprite(x,y,mario);
	display.update();
	color+=1;

	getch();

	boxc=mario;
	boxd.scale(boxc);
	boxe.scale(boxc);

	while(!kbhit() && c!=27) {
		display.syncsprites();
//		display.writef(40,24,7,"x %3d y %3d, x1 %3d y1 %3d, w %2d h %2d\n",x,y,x+dx,y+dy,w,h);
//		display.copyto(x,y,x+dx,y+dy,w,h);
//		boxd.drawbox(0,0,boxd.width()-1,boxd.height()-1,7,0);
		display.drawsprite(display.width()-x,display.height()-y,boxd);
		display.drawsprite(display.width()-x,y,boxe);
		display.drawsprite(x,y,boxc);
//		display.drawsprite(x,y,mario);
		//mario.rotate(boxc,rot);
//		box.rotate(boxc,rot);
		//boxd.scale(boxc);
		//boxe.scale(boxc);
//		boxc.drawbox(0,0,boxc.width(),boxc.height(),0,0);
//		c=getch();
		rot++;
		rot%=360;


		x+=dx;
		y+=dy;
		if(x>display.width()-w-1 || x<1) {
			box.drawbox(5,5,w-11,h-11,color);
			box.drawbox(6,6,w-13,h-13,color+8);
			box.drawbox(8,8,w-17,h-17,0);
			color+=1;
			dx*=-1;
		}
		if(y>display.height()-h-1 || y<1) {
			box.drawbox(5,5,w-11,h-11,color);
			box.drawbox(6,6,w-13,h-13,color+8);
			box.drawbox(8,8,w-17,h-17,0);
			color+=1;
			dy*=-1;
		}
		if (color > display.colors) color=1;

		display.update();
	}

	getch();

	display.textmode();

	display.debug();

	printf ("clear\n");


	getch();

	exit(0);
}
