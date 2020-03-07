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

	adapter *card = adapter::init(detect);
	adapter &display = *card;

	canvas &screen = display.screen;
	png	p;
	image img;

	display.graphmode(adapter::VGAHI);
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
	screen.drawimage(0,0,img);
	display.update();

//	display.getch();
	delete card;

	return 0;
}
