#include <string.h>
#include <stdio.h>
#include "image.h"
#include "vtext.h"
#include "types.h"

vtext::vtext()
{
	font_num=0;
	max_width=320;
	max_height=200;
}

vtext::vtext(int width, int height, unsigned char background)
{
	max_width=width;
	max_height=height;
	bgcolor=background;
	font_num=0;
	font=&fonts[font_num];
	debug(printf("init vtext fontwidth %d fontheight %d\n",font->font_width,font->font_height);)
}

vtext::~vtext()
{
}

bool vtext::drawtext(image& img, const char *string, unsigned char color)
{
	int width,height;
	int slen,x,y,x_cursor,y_cursor;
	int scount;
	unsigned char *cptr;
	unsigned char font_bits;
	char bitset=0;

	slen=strlen(string);

	width=(slen*font->font_width) % max_width;
	height=(((slen*font->font_width) / max_width) + 1) * font->font_height;

	debug(printf("setting width %d height %d\n",width,height);)
	if (!img.size(width,height)) {
		debug(printf("img.size failed\n");)
		return false;
	}

	x_cursor=0;
	y_cursor=0;
	for(scount=0;scount<slen;scount++){
		cptr=&font->font[string[scount]*font->bytes_per_char];
		debug(printf("cptr %c %02x xc %d yc %d\n",string[scount],*cptr,x_cursor,y_cursor);)
		for (y=0;y<font->font_height;y++) {
			font_bits=cptr[y];
			for (x=0;x<font->font_width;x++) {
				bitset=(font_bits>>(7-x&7)) & 0x1;
				debug(printf("%s",bitset?".":" ");)
				if (bitset) img.setpixel(x+x_cursor,y+y_cursor,color);
				else img.setpixel(x+x_cursor,y+y_cursor);
			}
			debug(printf("\n");)
		}

		x_cursor+=font->font_width;;
		x_cursor%=width;
		y_cursor=(scount*font->font_width)/width;
		debug(printf("scount %d sw %d w %d\n",scount,scount*font->font_width,(scount*font->font_width)/width);)
	}

	return true;
}
