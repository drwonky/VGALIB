#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"
#include "canvas.h"

#include "palettes.h"

class image : public canvas {

public:
image();
void printhex(void);
void dumppalette(void);

};

#endif
