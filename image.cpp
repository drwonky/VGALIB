#include <math.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "image.h"

#ifdef __BORLANDC__
#include <iostream.h>
#else
#include <iostream>
#endif

image::image()
	: canvas()
{
}

void image::printhex(void)
{
    int32_t i,j,b;

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
