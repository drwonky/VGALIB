#ifndef MYTYPES_H
#define MYTYPES_H

#ifdef DEBUG
#define debug(s) s
#else
#define debug(s)
#endif

#ifdef __BORLANDC__
//enum bool {false,true};
#define int32_t long
#define uint32_t unsigned long
#else
#define far 
#endif

#ifdef __GNUC__
#include <stdint.h>
#endif

#define bswap32(x) \
		((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |               \
		 (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define bswap16(x) \
		((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))

typedef unsigned char far * ptr_t;

#endif
