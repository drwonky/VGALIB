#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#define index 0x3b4
#define cntrl 0x3b8
#define scrn_on 8
#define grph 2
#define text 0x20

enum Boolean {false, true};

unsigned char gtable[]={0x35,0x2d,0x2e,0x07,
			0x5b,0x02,0x57,0x57,
			0x02,0x03,0x00,0x00};
unsigned char ttable[]={0x61,0x50,0x52,0x0f,
			0x19,0x06,0x19,0x19,
			0x02,0x0d,0x0b,0x0c};

class herc_routines {
private:
unsigned char far *screen;
unsigned int maxx_val, maxy_val;

public:
void graphmode(void);
void textmode(void);
void setpixel(int x, int y, unsigned char visible);
unsigned char getpixel(int x,int y);
void cls(void);
void tcls(void);
unsigned int maxx(void);
unsigned int maxy(void);
herc_routines(void);
};

herc_routines::herc_routines(void)
{
	screen=(unsigned char far *) MK_FP(0xb000,0);
	maxx_val=720;
	maxy_val=348;
}

void herc_routines::graphmode(void)
{
	unsigned char regnum;

	outportb(cntrl,grph);
	for(regnum=0;regnum<12;regnum++)
	{
		outportb(index,regnum);
		outportb(index+1,gtable[regnum]);
	}
	outportb(cntrl,grph+scrn_on);
}

void herc_routines::textmode(void)
{
	unsigned char regnum;

	outportb(cntrl,text);
	for(regnum=0;regnum<12;regnum++)
	{
		outportb(index,regnum);
		outportb(index+1,ttable[regnum]);
	}
	outportb(cntrl,text+scrn_on);
}

void herc_routines::setpixel(int x,int y, unsigned char visible)
{
	unsigned char temp_byte;
	unsigned int page_offset;

	page_offset=(0x2000*(y&3))+(90*(y/4))+(x/8);
	temp_byte=screen[page_offset];
	temp_byte=temp_byte&(255^(7-(x&7)));
	temp_byte=temp_byte|(visible<<(7-(x&7)));
	screen[page_offset]=temp_byte;
}

unsigned char herc_routines::getpixel(int x,int y)
{
	unsigned char temp_byte;
	unsigned int page_offset;

	page_offset=(0x2000*(y&3))+(90*(y/4))+(x/8);
	return((screen[page_offset]&(7-(x&7)))>>(7-(x&7)));
}

void herc_routines::cls(void)
{
	unsigned int scrn_offset;
	for(scrn_offset=0;scrn_offset<65535;scrn_offset++)
	{
		screen[scrn_offset]=0;
	}
}

void herc_routines::tcls(void)
{
	unsigned int scrn_offset;
	for(scrn_offset=0;scrn_offset<4000;scrn_offset+=2)
	{
		screen[scrn_offset]=0;
		screen[scrn_offset+1]=0x07;
	}
}

unsigned int herc_routines::maxx(void)
{
	return(maxx_val);
}

unsigned int herc_routines::maxy(void)
{
	return(maxy_val);
}

void main(void)
{
	unsigned int x,y;
	herc_routines display;

	display.graphmode();
	display.cls();
	for(y=0;y<display.maxy();y++)
	{
		for(x=0;x<display.maxx();x++)
		{
			display.setpixel(x,y,1);
		}
	}
	while (!kbhit());
	display.textmode();
	display.tcls();
}