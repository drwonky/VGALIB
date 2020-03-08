#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "png.h"
#include "vtext.h"
#include "adapter.h"
#include "ega.h"

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

//	adapter::Adapters card = adapter::detect();
//	printf("Detected %d\n",card);

	adapter *display = adapter::create(adapter::EGA);

	canvas &screen = display->screen;

	printf("About to go graphics...\n");
	display->graphmode(adapter::EGALO);
//	display->setpalette(palette::CGA1HI_PAL);
	canvas::setdefpalette(display->getpalette());
//	canvas::setdefpalette(palette::CGA1_PAL);

	image mario;
	image bg;

	printf("Loading images...\n");
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

//	display->graphmode(SDLVGALO);

//	display->initsprites();
	display->getch();
	display->cls();

	printf("Cleared\n");
	vtext text(screen.width(),screen.height(),0);
	mytext.setbg(0);
	text.drawtext(mytext,"Mario",1);

	png::loadimage("mariobg.png", bg);
//	screen.line(0,screen.height()-1,screen.width()-1,0,1);
	screen.drawsprite(display->width()-mytext.width(),2,mytext);

	printf("Sync...\n");

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
		screen.drawimage(320,0,bg);
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
	} while(!display->kbhit() && c!=27);

//	display->getch();

	delete display;

//	display->debuginfo();

	printf ("clear\n");


//	display->getch();

	return 0;
}
