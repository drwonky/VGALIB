#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

class memory {

public:
static void mask_memcpy(ptr_t dest, ptr_t src, size_t size, unsigned char mask);
static void fast_memcpy(ptr_t dest, ptr_t src, size_t size);
static void blit(ptr_t dest, ptr_t src, size_t size);

};

#endif
