#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"

#include "palettes.h"

class image {
public:
unsigned far char *buffer;

protected:
unsigned int w;
unsigned int h;
unsigned char bg;
int _palette_size;

palette::pal_t *_palette;

public:
image& operator = (const image& img);
image(const image& img);
image(void);
image(unsigned int width, unsigned int height);
image(unsigned int width, unsigned int height, unsigned char background);
~image();
void free(void);
palette::pal_t *getpalette(void) const { return _palette; }
int palette_size(void) const { return _palette_size; }
bool copypalette(const image& img);
void copypalette(palette::pal_t *p);
bool setpalette(palette::pal_type pal);
bool allocate(void);
ptr_t getbuffer(void) { return buffer; };
bool size(unsigned int width, unsigned int height);
unsigned char lookuppalentry(palette::pal_t *p);
unsigned char findnearestpalentry(palette::pal_t *p);
palette::pal_t getpalentry(int i);
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
void dumppalette(void);
int32_t wcolordist(palette::pal_t *a, palette::pal_t *b);
int32_t colordist(palette::pal_t *a, palette::pal_t *b);

};

#endif
