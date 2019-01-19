#ifndef PNG_H
#define PNG_H

#ifdef __BORLANDC__
#include <cstring.h>
#else
#include <string>
using namespace std;
#endif

#include <errno.h>

#include "image.h"

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
		unsigned char r;
		unsigned char g;
		unsigned char b;
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

class png
{
protected:
	union
	{ // Create a bunch of aliases to make buffer parsing easier
		char *_buffer;
		png_chunk *_chunk;
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
	unsigned char *_image_buffer; // Raw inflate() output

	int32_t _width;
	int32_t _height;
	char _depth;
	col_type _colors;
	char _compress;
	char _interlace;
	uint32_t _crc;
	int32_t _len;
	int _uncompressed_len;
	int _png_buf_size;
	int _scanline_size;
	int _bpp;

	uint32_t _ppu_x;
	uint32_t _ppu_y;
	char _unit;

	png_pal_entry *_pal;
	int _pal_size;

	png_bgtrns *_trns;
	int _trns_size;

	png_bgtrns _bg;

	enum png_errcodes { NONE,
					OPEN,			// Error opening file
					HEADER,			// Bad PNG header (corrupt or not PNG)
					ZLIB, 			// ZLIB error
					READ, 			// Error reading file
					CHUNK_SIZE, 	// Chunk size (other than IDAT) too big to fit in buffer
					BADALLOC, 		// Error allocating memory
					BADCRC 			// CRC error
	} _errno;

	static const char *_err_messages[];

private:
	png_blk_type png_block_name(png_chunk *chunk);
	void printhex(unsigned char *buf);
	bool allocate_img_buffer(void);
	int bytes_per_scanline(void);
	int paeth(unsigned char a, unsigned char b, unsigned char c);
	void filter(void);

public:
	png(void);
	~png(void);
	int32_t width(void) { return _width; }
	int32_t height(void) { return _height; }
	png_errcodes error(void) { return _errno; }
	string errormsg(void) {
			if (errno != 0) return string(_err_messages[_errno])+string(": ")+string(strerror(errno));
			else return string(_err_messages[_errno]);
	}
	bool load(const char *file);
	bool convert(image& img);
	void free(void);
};

#endif
