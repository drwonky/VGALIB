#ifndef PNG_H
#define PNG_H

#include "IMAGE.H"

#ifdef __GNUC__
#include <stdint.h>
#define long int32_t
#define ulong uint32_t
#else
#define int32_t long
#define uint32_t unsigned long
#include "TYPES.H"
#endif

#define bswap32(x) \
		((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |               \
		 (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define bswap16(x) \
		((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))

enum png_blk_type { eIHDR, ePLTE, eIDAT, eIEND, etRNS, egAMA, ecHRM, etEXt, esRGB, ebKGD, epHYs, esBIT, etIME, eiTXt, eUND };
static const char *png_blk_strings[] = { "IHDR","PLTE","IDAT","IEND","tRNS","gAMA","cHRM","tEXt","sRGB","bKGD","pHYs","sBIT","tIME","iTXt",0};
const unsigned char aPNG[] = { 0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A };

typedef volatile struct {
		unsigned char xbit;
		char PNG[3];
		char crlfzlf[4];
} png_signature;

typedef struct {
		int32_t len;
		char type[4];
} png_chunk;

enum col_type { GRAY=0, RGB=2, INDEXED=3, GRAYA=4, RGBA=6 };
typedef struct {
		png_chunk hdr;
		int32_t width;
		int32_t height;
		char depth;
		char ctype;
		char compress;
		char filter;
		char interlace;
		int32_t crc;
} png_IHDR;

typedef struct {
		unsigned char red;
		unsigned char green;
		unsigned char blue;
} png_pal_entry;

typedef struct {
		png_chunk hdr;
		png_pal_entry pal;
} png_PLTE;

typedef struct {
		char filter_type;
		unsigned char line;
} png_scanline;

typedef struct {
		png_chunk hdr;
		png_scanline lines;
} png_IDAT;

typedef struct {
		png_chunk hdr;
		int32_t crc;
} png_IEND;

typedef struct {
		short r;
		short g;
		short b;
} png_rgb;

typedef union {
		unsigned char ndx;
		short gray;
		png_rgb rgb;
} png_bgtrns;

typedef struct {
		png_chunk hdr;
		png_bgtrns alpha;
} png_tRNS;

typedef struct {
		png_chunk hdr;
		int32_t gamma;
} png_gAMA;

typedef struct {
		png_chunk hdr;
		int32_t wp_x;
		int32_t wp_y;
		int32_t red_x;
		int32_t red_y;
		int32_t grn_x;
		int32_t grn_y;
		int32_t blu_x;
		int32_t blu_y;
		int32_t crc;
} png_cHRM;

enum srgb_intent { PERCEP=0,RELATIVE=1,SAT=2,ABS=3 };

typedef struct {
		png_chunk hdr;
		char intent;
} png_sRGB;

typedef struct {
		png_chunk hdr;
		char text;
} png_tEXt;

typedef struct {
		png_chunk hdr;
		png_bgtrns bg;
} png_bKGD;  // crc omitted due to union

typedef struct {
		png_chunk hdr;
		int32_t ppu_x;
		int32_t ppu_y;
		char unit;
		int32_t crc;
} png_pHYs;

typedef struct {
		png_chunk hdr;
		union {
				char gray;
				png_pal_entry truecolor;
				png_pal_entry indexed;
				short graya;
				int32_t truecolora;
		};
} png_sBIT;  // crc omitted due to union

typedef struct {
unsigned char h:4;
unsigned char l:4;
} png_4bpp;

typedef struct {
unsigned char a:2;
unsigned char b:2;
unsigned char c:2;
unsigned char d:2;
} png_2bpp;

typedef struct {
unsigned char a:1;
unsigned char b:1;
unsigned char c:1;
unsigned char d:1;
unsigned char A:1;
unsigned char B:1;
unsigned char C:1;
unsigned char D:1;
} png_1bpp;

class png {
protected:
union { // Create a bunch of aliases to make buffer parsing easier
char *buffer;
png_chunk *chunk;
png_IHDR *IHDR;
png_PLTE *PLTE;
png_IDAT *IDAT;
png_IEND *IEND;
png_tRNS *tRNS;
png_gAMA *gAMA;
png_cHRM *cHRM;
png_sRGB *sRGB;
png_tEXt *tEXt;
png_bKGD *bKGD;
png_pHYs *pHYs;
png_sBIT *sBIT;
};

public:
unsigned char *image_buffer; // Raw inflate() output

int32_t width;
int32_t height;
char depth;
col_type colors;
char compress;
char interlace;
int32_t crc;
int32_t len;
int	uncompressed_len;
int	png_buf_size;
int scanline_size;

uint32_t ppu_x;
uint32_t ppu_y;
char unit;

png_pal_entry *pal;
int pal_size;

png_bgtrns *trns;
int trns_size;

png_bgtrns bg;

private:
png_blk_type png_block_name(png_chunk *chunk);
void printhex(unsigned char *buf);
bool allocate_img_buffer(void);
int bytes_per_scanline(void);

public:
png(void);
~png(void);
bool load(char *file);
bool convert2image(image& img);
void free(void);
};

#endif
