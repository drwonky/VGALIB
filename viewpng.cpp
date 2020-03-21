#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "vgalib.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;
#endif

int main(int argc, char *argv[])
{

	adapter::Adapters detect = adapter::detect();
	printf("Detected %d\n",detect);

	adapter *card = adapter::create(detect);
	adapter &display = *card;

	canvas &screen = display.screen;
	png	p;
	image img;
	int xpos=0;
	int ypos=0;

	display.graphmode(adapter::FHD);
	display.setpalette(palette::RGB_PAL);
	canvas::setdefpalette(display.getpalette());
	display.cls();

	printf("Loading...\n");
	if (!p.load(argv[1])) {
		cout<<"Error loading PNG: "<<p.errormsg()<<endl;
		return 1;
	}

	p.convert(img);

	printf("Image width %d height %d\n",img.width(),img.height());
	int viewport_width,viewport_height;

	viewport_width = img.width() > screen.width() ? screen.width() : img.width();
	viewport_height = img.height() > screen.height() ? screen.height() : img.height();
	screen.drawimage(0,0,xpos,ypos,viewport_width,viewport_height,img);
	display.update();

	int ch=0;
	int xincrement=img.width()/100;
	int yincrement=img.height()/100;

	while((ch=display.getch()) != 'q' && ch != 0) {
		if (img.width() > screen.width() || img.height() > screen.height()) {
		switch (ch) {
		case 119:
			ypos = ypos>yincrement ? ypos-yincrement : 0;
			break;
		case 115:
			if (viewport_height < img.height())
				ypos = ypos+yincrement<img.height()-screen.height() ? ypos+yincrement : img.height()-screen.height();
			break;
		case 97:
			xpos = xpos>xincrement ? xpos-xincrement : 0;
			break;
		case 100:
			if (viewport_width < img.width())
				xpos = xpos+xincrement<img.width()-screen.width() ? xpos+xincrement : img.width()-screen.width();
			break;
		}
		}
		cout<<"got "<<(int)ch<<" xpos "<<xpos<<" ypos "<<ypos<<endl;
		screen.drawimage(0,0,xpos,ypos,viewport_width,viewport_height,img);
		display.update();
	}

	delete card;

	return 0;
}
