#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "png.h"
#include "vtext.h"
#include "vga.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;
#endif

vga::vga(void)
{
	unsigned char mode;
	saved_buffer = new unsigned far char[1];
#ifndef SDL
	os_buffer = new unsigned far char[1];
#else
	os_buffer=NULL;
	spritescreen=NULL;
#endif
	sprite_buffer = new unsigned far char[1];
	sprites=0;

	colors=0;
	_width=0;
	_height=0;
	mpalette=NULL;
	mpalette_size=0;
	row_bytes=0;
	bpp=0;
	Bpp=0;
	screen=NULL;
	SR=0;
	planes=0;
	buffer=NULL;
	buf_size=0;

#ifdef __BORLANDC__
	getmode();

	if (vmode == MDA) {
		card = MONO;
	} else {
		card = COLOR;
	}
#else
	vmode = SDLVGALO;

	if (!SDLonce) {
		SDLonce=true;
		SDL_Init(SDL_INIT_VIDEO);
//		SDL_GL_SetSwapInterval(1);
	}

	SDL_WM_SetCaption("VGA", "VGA");
	offscreen=NULL;
	card=COLOR;
#endif

}

vga::~vga(void)
{
	printf("vga destructor called\n");
	delete[] saved_buffer;
	delete[] sprite_buffer;
#ifdef SDL
	if (offscreen) SDL_FreeSurface(offscreen);
	if (SDLonce) {
		SDL_Quit();
	}
#else
	delete[] os_buffer;
#endif
}

void vga::debuginfo(void)
{
	printf("vmode: %d\n", vmode);
	printf("buffer: %Fp\n", buffer);
	printf("os buffer: %Fp\n", os_buffer);
	printf("sprite buffer: %Fp\n", sprite_buffer);
	printf("_width: %d\n",_width);
	printf("_height: %d\n",_height);
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

#ifndef SDL
	delete[] os_buffer;
#endif
	for (i=0;i<sizeof(video_modes);i++) {
		if (video_modes[i].mode == vmode) {
			row_bytes = video_modes[i].bytes;
			_width = video_modes[i].x;
			_height = video_modes[i].y;
			bpp = video_modes[i].bpp;
			Bpp = bpp/8;
			planes = video_modes[i].planes;
			colors = video_modes[i].colors;
			SR = video_modes[i].sr;
#ifdef __BORLANDC__
			buffer = (unsigned char far *) MK_FP(video_modes[i].fp,0);
#else
#endif
			buf_size = _width*_height;
#ifdef SDL
			offscreen = SDL_CreateRGBSurface(0,_width,_height,8,0,0,0,0);
			os_buffer = (unsigned char *)offscreen->pixels;

			spritescreen=SDL_CreateRGBSurface(0,_width,_height,8,0,0,0,0);
			sprite_buffer=(unsigned char *)spritescreen->pixels;
#else
			os_buffer = allocate_screen_buffer();
#endif
			return true;
		}
	}
	return false;
}

bool vga::setpalette(pal_type pal)
{
	mpalette_size=palettes[pal].palette_entries;
	mpalette=new img_pal[mpalette_size];

	if (mpalette == NULL) return false;

	memcpy(mpalette,palettes[pal].pal,sizeof(img_pal)*mpalette_size);

#ifdef SDL
	SDL_Color* sdl_pal_colors = new SDL_Color[mpalette_size];

	for(int i=0;i<mpalette_size;i++) {
		sdl_pal_colors[i].r=mpalette[i].r;
		sdl_pal_colors[i].g=mpalette[i].g;
		sdl_pal_colors[i].b=mpalette[i].b;
	}

	SDL_SetPalette(screen,SDL_PHYSPAL,sdl_pal_colors,0,mpalette_size);
	SDL_SetPalette(offscreen,SDL_LOGPAL,sdl_pal_colors,0,mpalette_size);
	SDL_SetPalette(spritescreen,SDL_LOGPAL,sdl_pal_colors,0,mpalette_size);

	delete[] sdl_pal_colors;
#endif

	return true;
}

void vga::initsprites(void)
{
#ifndef SDL
	sprite_buffer=allocate_screen_buffer();
#endif
	cls(sprite_buffer);
	sprites=1;
}

bool vga::graphmode(void)
{
#ifdef __BORLANDC__
	vmode = VGAMONO;
	_AL=(unsigned char) VGAMONO;
	_AH=0;
	geninterrupt(0x10);
	return setup();
#else
	return true;
#endif
}

bool vga::sdlmode(void)
{
#ifdef SDL
	vmode=SDLVGALO;
	setup();

	screen = SDL_SetVideoMode(_width, _height, 0, 0);
	buffer = (unsigned char *)screen->pixels;

	if (screen != NULL) {
		setpalette(VGA_PAL);
		return true;
	}

	return false;
#else
	return true;
#endif
}

bool vga::graphmode(Vgamode mode)
{
	switch (mode) {
		case SDLVGALO:
			return sdlmode();
		case MDA:
			return mdamode();
		case TEXT:
			return textmode();
		case X16:
			if (card == MONO) return mdamode();
			return x16mode();
	}

#ifdef __BORLANDC__
	vmode=mode;
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
	if (card == MONO) {
		vmode = MDA;
	} else {
		vmode = TEXT;
	}

#ifdef __BORLANDC__
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
#ifdef __BORLANDC__
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
#ifdef __BORLANDC__
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
#ifdef __BORLANDC__
	outportb(port, reg);
	outportb(port+1,val);
#endif
}

Vgamode vga::getmode(void)
{
#ifdef __BORLANDC__
	_AX=0x0f00;
	geninterrupt(0x10);
	vmode=(Vgamode)_AL;
	return (vmode);
#else
#ifdef SDL
	return SDLVGALO;
#endif
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
		case SDLVGALO:
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
		case SDLVGALO:
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
		case SDLVGALO:
		case VGAMONO:
			memset((void *)buf,0,buf_size);
			break;
		case VGAHI:

			break;

	}
}

unsigned int vga::width(void)
{
	return(_width);
}

unsigned int vga::height(void)
{
	return(_height);
}

void vga::save_buffer(void)
{
	delete saved_buffer;

	saved_buffer = allocate_screen_buffer();
	switch (vmode) {
		case SDLVGALO:
		case VGALO:
		case VGAMONO:
		case TEXT:
#ifdef __BORLANDC__
			_fmemcpy(saved_buffer,&buffer,row_bytes*_height*planes);
#else
			memcpy(saved_buffer,&buffer,row_bytes*_height*planes);
#endif
			break;
		case VGAHI:
#ifdef __BORLANDC__
			_fmemcpy(saved_buffer,&buffer,row_bytes*_height*planes);
#else
			memcpy(saved_buffer,&buffer,row_bytes*_height*planes);
#endif
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

	if (x1>_width-1 || y1>_height) return;

	w=x1+w > _width ? _width-x1 : w;
	h=y1+h > _height ? _height-y1 : h;
	switch (vmode) {
		case X16:
		case MDA:
		case SDLVGALO:
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
#ifdef __BORLANDC__
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
#else
	if (sprites) {
		SDL_BlitSurface(spritescreen, NULL, screen, NULL);
	} else {
		SDL_BlitSurface(offscreen, NULL, screen, NULL);
	}
	SDL_UpdateRect(screen, 0, 0, 0, 0);
#endif
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

	if (x>_width-1 || y>_height-1 || x<0 || y<0) return;

	w=x+w > _width-1 ? _width-x : w;
	h=y+h > _height-1 ? _height-y : h;
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

	if (x>_width-1 || y>_height-1 || x<0 || y<0) return;

	w=x+w > _width-1 ? _width-x : w;
	h=y+h > _height-1 ? _height-y : h;
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
#ifdef __BORLANDC__
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
#else
	// TODO SDL_GL_SetSwapInterval()
#endif
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

bool vga::kbhit(void)
{
	if (SDL_PollEvent(&event)) {
		/* an event was found */
		switch (event.type) {
		/* close button clicked */
		case SDL_QUIT:
			return true;
			break;

			/* handle the keyboard */
		case SDL_KEYDOWN:
			return true;
			break;
		}
	}

	return false;
}

char vga::getch(void)
{
	if (SDL_WaitEvent(&event)) {
		/* an event was found */
		switch (event.type) {
		/* close button clicked */
		case SDL_QUIT:
			return 0;
			break;

			/* handle the keyboard */
		case SDL_KEYDOWN:
			return event.key.keysym.sym;
			break;
		}
	}

	return 0;
}

int main(void)
{
	int x,y,w,h,dx,dy;
	uint32_t i;
	unsigned char color=1;
	unsigned char c=0;
	unsigned int rot=0;
	vga display;
	png mariopng;
	image mario;

	w=20;
	h=20;
	dx=1;
	dy=1;
	x=0;
	y=0;

	printf("Starting...\n");

	display.graphmode(SDLVGALO);
	display.initsprites();
	display.cls();

	mariopng.load("mario16.png");

	mario.setpalette(VGA_PAL);
	mariopng.convert2image(mario);
	mariopng.free();

	mario.printhex();
	w=mario.width();
	h=mario.height();

	image box(w,h);
	image boxc(w,h);
	image boxd(w*2,h*2);
	image boxe(w/2,h/2);
	image mytext;
	mytext.setpalette(VGA_PAL);

	vtext text(display.width(),display.height(),0);
	text.drawtext(mytext,"mario",7);

	box.drawbox(5,5,w-11,h-11,color);
	box.drawbox(6,6,w-13,h-13,color+8);
	box.drawbox(8,8,w-17,h-17,0);

//	display.getch();
//	display.debuginfo();

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

//	display.getch();

	boxc=mario;
	boxd.scale(boxc);
	boxe.scale(boxc);

	display.drawsprite(x,y,mytext);

	while(!display.kbhit() && c!=27) {
		display.syncsprites();
//		display.writef(37,0,7,"rot %d\n",rot);
//		display.writef(40,24,7,"x %3d y %3d, x1 %3d y1 %3d, w %2d h %2d\n",x,y,x+dx,y+dy,w,h);
//		display.copyto(x,y,x+dx,y+dy,w,h);
//		boxd.drawbox(0,0,boxd.width()-1,boxd.height()-1,7,0);
		mario.rotate(boxc,rot);
		boxd.scale(boxc);
		boxe.scale(boxc);
		display.drawsprite(display.width()-x,display.height()-y,boxd);
		display.drawsprite(display.width()-x,y,boxe);
		display.drawsprite(x,y,boxc);
		display.drawsprite(x,y+boxc.height(),mytext);
//		display.drawsprite(x,y,mario);
//		box.rotate(boxc,rot);
//		boxc.drawbox(0,0,boxc.width()-1,boxc.height()-1,0,0);
//		c=display.getch();
#ifdef SDL
		SDL_Delay(1000/60);
#endif
		display.update();
		rot++;
		if(rot>360) rot=0;


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

	}

//	display.getch();

	display.textmode();

	display.debuginfo();

	printf ("clear\n");


//	display.getch();

	return 0;
}
