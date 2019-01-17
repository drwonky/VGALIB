#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"
#include "canvas.h"

#include "palettes.h"

class image : public canvas {

public:
image(void);
image(unsigned int width, unsigned int height);
image(unsigned int width, unsigned int height, unsigned char background);
void printhex(void);
void dumppalette(void);

};

#endif
