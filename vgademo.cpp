#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "png.h"
#include "vtext.h"
#include "adapter.h"
#include "sdl.h"
#include "vga.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;
#endif

int main(void)
{
	int x,y,w,h,dx,dy;
	unsigned char c=0;
	int rot=0,rotx=(rand()%5+1);

	dx=1;
	dy=1;
	x=0;
	y=0;

	printf("Starting...\n");

	adapter::Adapters card = adapter::detect();
	printf("Detected %d\n",card);

	adapter *display = adapter::init(card);

	canvas &screen = display->screen;

//	display->graphmode(SDLVGALO);
	display->graphmode(adapter::VGALO);

//	display->initsprites();
	display->cls();
	display->setpalette(palette::VGA_PAL);
	canvas::setdefpalette(display->getpalette());

	image mario;
	image bg;

	printf("Loading...\n");
	png::loadimage("mario16.png", mario);

	w=mario.width();
	h=mario.height();

//	mario.setpalette(display->getpalette());
	printf("Mario width %d height %d\n",mario.width(),mario.height());
//	mario.setbg(254);

//	mario.printhex();
	printf("Math...\n");
	double sides=(double)mario.width()*(double)mario.width()+(double)mario.height()*(double)mario.height();
	int hyp=(int) sqrt(sides);
	w=hyp;
	h=hyp;

	printf("Building...\n");
	image box(w,h);
	image boxc(w,h);
	image boxd(w*2,h*2);
	image boxe(w/2,h/2);
	image mytext;
//	mytext.setpalette(display->getpalette());

	vtext text(display->width(),display->height(),0);
	mytext.setbg(14);
	text.drawtext(mytext,"Mario",4);

//	mariopng.loadimage("mariobg.png", bg);
	png::loadimage("mariobg.png", bg);
//	mariopng.convert(bg);
//	mariopng.free();
	screen.drawimage(0,0,bg);
//	bg.free();

/*
	for(x=0;x<display->width();x++) {
		for(y=0;y<display->height();y++) {
			display->setpixel(x,y,(x/(display->width()/display->colors)));
		}
	}
*/

	printf("Sync...\n");
//	display->syncsprites();
	display->update();

	x=y=0;
//	display->syncsprites();

//	box.setpalette(display->getpalette());
	box.copypalette(mario);
	boxc.copypalette(mario);
	boxd.copypalette(mario);
	boxe.copypalette(mario);
	box.setbg(mario.getbg());
	boxc.setbg(mario.getbg());
	boxd.setbg(mario.getbg());
	boxe.setbg(mario.getbg());
	box.clear();
	box.drawimage((box.width()-mario.width())/2,(box.height()-mario.height())/2,mario);

	boxc=box;
	boxd.scale(boxc);
	boxe.scale(boxc);
//	display->drawsprite(x,y,boxc);

//	display->drawsprite(x,y,boxd);
//	display->drawsprite(x,y,boxe);
//	display->drawsprite(display->width()-mytext.width(),2,mytext);
//	display->getch();
//	display->update();
//	color+=1;

//	display->getch();

	printf("Begin..\n");
	do {
//		display->syncsprites();
		screen.drawimage(0,0,bg);
		screen.drawsprite(display->width()-mytext.width(),2,mytext);
		box.rotate(boxc,rot);
//		if (rotframes==2) rotframes=0,box.rotate(boxc,rot);
//		rotframes++;
//		boxd.scale(boxc);
//		boxe.scale(boxc);
//		display->drawsprite(x,y,boxd);
		screen.drawsprite(x,y,boxc);
//		display->drawsprite(x,y,boxe);


		display->update();
		rot+=rotx;
		if(rot>360) rot=0;
		if(rot<0) rot=360;

		x+=dx;
		y+=dy;

		if(x>display->width()-w-1 || x<1) {
			dx*=-1;
			rotx*=-1;
			rotx=((rotx>0)-(rotx<0))*(rand()%5+1);
		}

		if(y>display->height()-h-1 || y<1) {
			dy*=-1;
			rotx*=-1;
			rotx=((rotx>0)-(rotx<0))*(rand()%5+1);
		}
	} while(!KBHIT() && c!=27);

//	display->getch();

	display->textmode();

//	display->debuginfo();

	printf ("clear\n");


//	display->getch();

	return 0;
}
