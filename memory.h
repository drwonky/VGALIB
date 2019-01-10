#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

class memory {

public:
static void mask_memcpy(unsigned char far *dest, unsigned char far *src, size_t size, unsigned char mask);
static void fast_memcpy(unsigned char far *dest, unsigned char far *src, size_t size);
static void blit(unsigned char far *dest, unsigned char far *src, size_t size);

};

#endif
