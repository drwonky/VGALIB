#include <string.h>
#include "types.h"

class tile {
protected:

int tilewidth;
int tileheight;
int tilesize;
char tilebpp;

unsigned char *tiledata;

};

tile::tile()
{
	tilewidth=0;
	tileheight=0;
	tilesize=0;
	tilebpp=0;
	tiledata=NULL;
}

tile::~tile()
{
	if(tiledata) delete[] tiledata;
}

tile::tile(int width, int height, char bpp, unsigned char *data)
{
	tilesize=width*height*bpp/8;
	tilewidth=width;
	tileheight=height;
	tilebpp=bpp;

	tiledata=new unsigned char[tilesize];

	if (tiledata == NULL) return;

	memcpy(tiledata,data,tilesize);
}

uint32_t tile::getpixel(int x, int y)
{
	uint32_t ret;

	switch(tilebpp) {
		case 1:
			ret=(tiledata[y*(tilewidth/8) + (x/8)];
			ret=(ret >> (x&7)) & 0x01;
			break;
		case 2:
			ret=tiledata[y*(tilewidth/4) + (x/4)];
			ret=(ret >> (x&2*6)) & 0x03;
			break;
		case 4:
			ret=tiledata[y*(tilewidth/2) + (x/2)];
			ret=(ret >> (x&1*4)) & 0x0f;
			break;
		case 8:
			ret=tiledata[y*tilewidth + x];
			break;
		case 16:
			ret=tiledata[y*tilewidth*2 + x*2];
			break;
		case 24:
			ret=tiledata[y*tilewidth*3 + x*3];
			break;
		case 32:
			ret=tiledata[y*tilewidth*4 + x*4];
			break;
	}

	return ret;
}
