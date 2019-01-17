#include <math.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "image.h"

#include <iostream>

image::image(void)
	: canvas()
{
	setpalette(palette::VGA_PAL);
}

image::image(unsigned int width, unsigned int height)
	: canvas(width,height)
{
	setpalette(palette::VGA_PAL);
	size(width,height);
}

image::image(unsigned int width, unsigned int height, pixel_t background)
	: canvas(width,height)

{
	setpalette(palette::VGA_PAL);
	size(width,height);
	setbg(background);
}

void image::printhex(void)
{
    unsigned int i,j,b;

    b=_width;

	printf("width: %d height: %d\n",_width,_height);
    printf("    ");
    for (i=0;i<b;i++) {
        printf("%2d ",i);
    }
    printf("\n");
    for (j=0;j<_height;j++) {
        printf("%2d: ",j);
        for (i=0;i<b;i++) {
            printf("%02x ",_buffer[j*_width+i]);
        }
        printf("\n");
    }
}

void image::dumppalette(void)
{
	int i;

	for (i=0;i<_palette_size;i++) {
		printf("%d: %02x %02x %02x\n",i,_palette[i].r,_palette[i].g,_palette[i].b);
	}
}

/*
void image::drawtile(image& image, int x, int y, char bpp, unsigned char color, unsigned char *tiledata)
{
	int tx,ty;

	switch(bpp) {
		case 1:
			for(tx=0;tx<
*/
