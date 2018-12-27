#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <mem.h>
#define VGAMONO 0x11
#define TEXTMODE 0x03
#define GRAPH 1
#define TEXT 0

class vga_mono {
protected:
unsigned char far *screen;
unsigned char far *sbuffer;
unsigned int far *tscreen;
unsigned int *tsvscrn;
unsigned int max_x;
unsigned int max_y;
char gr_or_txt;
unsigned char temp;
unsigned int p_o;
unsigned char bit;
unsigned int yhash[480];

public:
vga_mono(void);
char graphmode(void);
char textmode(void);
void setpixel(int x, int y, char visible);
char getpixel(int x, int y);
void cls(void);
unsigned int maxx(void);
unsigned int maxy(void);
void tsave(void);
void trestore(void);
};

class bitmaps {
unsigned char *bitmap;
unsigned long width;
unsigned long r_width;
unsigned long height;
unsigned long size;
unsigned long pixels;

public:
void getbitmap(char *filename);
void blitmap(vga_mono &disp, int x, int y);
~bitmaps(void);
void flip(void);
};

void exitinstr(char *err);

vga_mono::vga_mono(void)
{
	screen=(unsigned char far *) MK_FP(0xa000,0);
	tscreen=(unsigned int far *) MK_FP(0xb800,0);
//	if((tsvscrn=(unsigned int *) malloc(4000))==NULL) exitinstr("Error: can't allocate buffer to save screen\n");
	max_x=640;
	max_y=480;
	gr_or_txt=TEXT;
	for(p_o=0;p_o<480;p_o++)
	{
		yhash[p_o]=p_o*80;
	}
}

char vga_mono::graphmode(void)
{
	_AL=VGAMONO;
	_AH=0;
	geninterrupt(0x10);
	gr_or_txt=GRAPH;
	return(1);	//nothing to return yet, just for future
}

char vga_mono::textmode(void)
{
	_AL=TEXTMODE;
	_AH=0;
	gr_or_txt=TEXT;
	geninterrupt(0x10);
	return(1);
}

void vga_mono::setpixel(int x, int y, char visible)
{
	p_o=(yhash[y])+(x>>3);
	bit=(7-x&7);
	temp=screen[p_o];
	temp=temp&(255^(1<<bit));
	temp=temp|(visible<<bit);
	screen[p_o]=temp;
}

char vga_mono::getpixel(int x, int y)
{
	return((screen[(y*80)+(x/8)]&(7-(x&7)))>>(7-(x&7)));
}

void vga_mono::cls(void)
{
	unsigned long temp;
	if (gr_or_txt==GRAPH)
	{
		for(temp=0;temp<38400L;temp++)
		{
			screen[temp]=0;
		}
	}
	else
	{
		for(temp=0;temp<2000;temp++)
		{
			tscreen[temp]=0x720;
		}
	}
}

unsigned int vga_mono::maxx(void)
{
	return(max_x);
}

unsigned int vga_mono::maxy(void)
{
	return(max_y);
}

void vga_mono::tsave(void)
{
	unsigned int temp;
	for(temp=0;temp<4000;temp++)
	{
		tsvscrn[temp]=tscreen[temp];
	}
}

void vga_mono::trestore(void)
{
	unsigned int temp;
	for(temp=0;temp<4000;temp++)
	{
		tscreen[temp]=tsvscrn[temp];
	}
}

void bitmaps::blitmap(vga_mono &disp, int x, int y)
{
	unsigned int x1,y1,tx=(x+width),ty=(y+height);
	unsigned int maxx=disp.maxx(),maxy=disp.maxy();
	for(y1=y;y1<ty&&y1<maxy;y1++)
	{
		for(x1=x;x1<tx&&x1<maxx;x1++)
		{
			disp.setpixel(x1,y1,bitmap[((y1-y)*(r_width))+(x1-x)]);
		}
	}
}

void bitmaps::getbitmap(char *filename)
{
	unsigned long temp,temp1;
	unsigned int bpp;
	unsigned char *tc;
	FILE *in;
	if((in=fopen(filename,"rb"))==NULL)
	{
		exitinstr("Error opening BMP file\n");
	}
	fseek(in,28L,0);
	fread(&bpp,2,1,in);
	if(bpp!=1)
	{
		exitinstr("Error: BMP must be 1 bit per pixel\n");
	}
	fseek(in,18L,0);
	fread(&width,4,1,in);
	fread(&height,4,1,in);
	r_width=(((width+31)/32)*32);
	pixels=r_width*height;
	size=(pixels+7)/8;
	size=(size+3)/4;
	size*=4;
	temp=pixels;temp1=size;
	if((bitmap=(unsigned char *) malloc(size*8))==NULL)
	{
		printf("Error allocating %lu bytes for BMP\n",size*8);
		exitinstr("");
	}
	tc=bitmap;
	fseek(in,62L,0);

	while (temp > 0)
	{
		temp1--;
		int x=fgetc(in);
		int m=0x80;
		while((temp>0) && (m!=0))
		{
			tc[0]=(x & m) ? 1 : 0;
			tc++;
			m >>=1;
			temp--;
		}
	}
	while (temp1-- >0) fgetc(in);
	printf("%lu (%lu) x %lu = %lu (%lu)\n",width,r_width,height,size,(r_width*height)/8);
	delay(200);
	fclose(in);
}

void bitmaps::flip(void)
{
	unsigned long y;
	unsigned char *swap;

	swap=new unsigned char [r_width];
	for(y=0;y<(height/2);y++)
	{
			memcpy(swap,&bitmap[y*r_width],r_width);
			memcpy(&bitmap[y*r_width],&bitmap[(height-y)*r_width],r_width);
			memcpy(&bitmap[(height-y)*r_width],swap,r_width);
	}
	delete swap;
}

bitmaps::~bitmaps(void)
{
	free(bitmap);
}

void wait(void)
{
	getch();
}

void exitinstr(char *err)
{
	printf("%s\n",err);
	printf("APSoft logo screen saver, (C) 1995 APSoft \n\
USAGE: APSlogo -d delay -f filename \n\
	delay: a number from 1 to 9 \n\
	filename: alternate BMP file \n");
	exit(1);
}

void main(int argc, char *argv[])
{
	int x,y,delfact=5;
	char *filename="apsoft.bmp",*argptr,args;
	vga_mono display;
	bitmaps aps;

	if (argc)
	{
		for(args=1;args<argc+1;args++)
		{
			argptr=argv[args];
			if(argptr[0]=='-')
			{
				switch(argptr[1])
				{
					case 'd':
					if((delfact=atoi(argv[args+1]))==0)
					{
						exitinstr("Wrong delay value");
					}
					break;
					case 'f':
					if((filename=argv[args+1])==NULL)
					{
						exitinstr("Bad filename");
					}
					break;
					default:
					exitinstr("Bad argument");
					break;
				}
			}
		}
	}
	aps.getbitmap(filename);
    aps.flip();
	display.graphmode();
	display.cls();
	while(!kbhit())
	{
		delay(delfact*100);
		display.cls();
		aps.blitmap(display, random(640),random(480));
	}

	display.textmode();
	display.cls();
}