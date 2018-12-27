#ifndef IMAGE_H
#define IMAGE_H

#include "TYPES.H"

typedef struct { unsigned char r; unsigned char g; unsigned char b; } img_pal;

#include "vga_pal.h"

const int palette_size=256;

class image {
public:
unsigned far char *buffer;

protected:
unsigned int w;
unsigned int h;
unsigned char bg;

img_pal *mpalette;

public:
image& operator = (const image& img);
image(const image& img);
image(void);
image(unsigned int width, unsigned int height);
image(unsigned int width, unsigned int height, unsigned char background);
~image();
void free(void);
img_pal *palette(void) const { return mpalette; }
void copypalette(img_pal *p);
bool allocate(void);
bool size(unsigned int width, unsigned int height);
int mappalentry(img_pal *p);
img_pal getpalentry(int i);
void setbg(unsigned char background);
unsigned char getbg(void);
unsigned int width(void);
unsigned int height(void);
void clear(unsigned char color);
void clear(void);
void setpixel(unsigned int x, unsigned int y);
void setpixel(unsigned int x, unsigned int y, unsigned char color);
unsigned char getpixel(unsigned int x, unsigned int y);
void drawbox(int x,int y,int width,int height,unsigned char color);
void drawbox(int x, int y, int width, int height, unsigned char color, int filled);
void line(int x0, int y0, int x1, int y1, unsigned char color);
void copyto(unsigned int x, unsigned int y, unsigned int x1, unsigned int y1, unsigned int width, unsigned int height);
void copyto(image far *src, image far *dest, unsigned int sx, unsigned int sy, unsigned int dx, unsigned int dy, unsigned int width, unsigned int height);
void rotate(image& dest, int angle);
void rotate(int angle);
void scale(image& dest, int width, int height);
image& scale(int width, int height);
void scale(image& img);
void printhex(void);

};

#endif
