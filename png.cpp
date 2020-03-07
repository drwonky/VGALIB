#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "image.h"
#include "types.h"

#ifdef __BORLANDC__
#include <fstream.h>
#include <iostream.h>
#include "zlib.h"
#else
#include <fstream>
#include <iostream>
#include <zlib.h>

using namespace std;

#endif

#include "png.h"

const char *png_blk_strings[] =
{ "IHDR", "PLTE", "IDAT", "IEND", "tRNS", "gAMA", "cHRM", "tEXt", "sRGB",
		"bKGD", "pHYs", "sBIT", "tIME", "iTXt", 0 };


#define BUFSIZE 8192

const char *png::_err_messages[] = {
"None",
"Error opening file",
"Bad PNG header (corrupt or not PNG)",
"ZLIB error",
"Error reading file",
"Chunk size (other than IDAT) too big to fit in buffer",
"Error allocating memory",
"CRC error"
};

png::png(void)
{

	_image_buffer=NULL;
	_buffer=NULL;
	_pal=NULL;
	_trns=NULL;
	_pal_size=0;
	_trns_size=0;
	_width=0;
	_height=0;
	_colors=GRAY;
	_depth=0;
	_interlace=0;
	_compress=0;
	_bpp=0;
	_ppu_x=0;
	_ppu_y=0;
	_unit=0;
	_scanline_size=0;
	_len=0;
	_crc=0;
	_png_buf_size=0;
	_uncompressed_len=0;
	_errno=NONE;

}

png::~png(void)
{
	this->free();
}

string png::errormsg(void)
{
		if (errno != 0) return string(_err_messages[_errno])+string(": ")+string(strerror(errno));
		else return string(_err_messages[_errno]);
}

void png::free(void)
{
	if (_image_buffer) { delete[] _image_buffer; _image_buffer=NULL; }
	if (_pal) { delete[] _pal; _pal=NULL; }
	if (_trns) { delete[] _trns; _trns=NULL; }
	if (_buffer) { delete[] _buffer; _buffer=NULL; }
}

png_blk_type png::png_block_name(png_chunk *chunk)
{
	int i=0;

	while(png_blk_strings[i]) {
		if (memcmp(chunk->type,png_blk_strings[i],4)==0) {
			return (png_blk_type)i;
		}
		i++;
	}

	return eUND;
}

void png::printhex(unsigned char *buf)
{
	int i,j,b;

	b=bytes_per_scanline()+1;

	printf("%d bytes per scanline\n",b);

	printf("    ");
	for (i=0;i<b;i++) {
		printf("%2d ",i);
	}
	printf("\n");
	for (j=0;j<_height;j++) {
		printf("%2d: ",j);
		for (i=0;i<b;i++) {
			printf("%02x ",buf[(j*_uncompressed_len/_height)+i]);
		}
		printf("\n");
	}
}

int png::bytes_per_scanline(void)
{
	switch(_colors) {
		case GRAY:
		case INDEXED:
			return ((_width*_depth)+(_width*_depth%8))/8;
		case RGB:
			return _width*_depth*3/8;
		case GRAYA:
			return _width*(_depth/8+1);
		case RGBA:
			return _width*(_depth*3/8+1);
	}

	return 0;
}

bool png::allocate_img_buffer(void)
{
	_png_buf_size=_scanline_size=bytes_per_scanline();

	debug(cout<<"Calculated "<<_png_buf_size<<" per line"<<endl;)
	_png_buf_size++; //filter mode at begin of each scanline

	_png_buf_size*=_height;

	debug(cout<<"Allocated "<<_png_buf_size<<" bytes for image buffer"<<endl;)

	_image_buffer = new unsigned char[_png_buf_size];

	if (_image_buffer != NULL) return true;
	return false;
}

bool png::load(const char *file)
{
	free(); // save caller some time

	if (!_buffer) _buffer = new char[BUFSIZE];

	if (!_buffer) {
		cerr << "Error allocating file buffer"<<endl;
		return false;
	}

	ifstream in;

	debug(cout<<"Loading "<<file<<endl;)
	in.open(file,fstream::binary);

	if (!in) {
		cerr << "__FILE__:__LINE__ error opening" << file << endl;
		_errno = OPEN;
		return false;
	}

    in.seekg (0, in.end);
    _len = in.tellg();
    in.seekg (0, in.beg);
	_uncompressed_len=0;

	debug(cout<<"File length "<<_len<<endl;)

	in.read(_buffer,sizeof(png_signature));

	if (memcmp(_buffer,aPNG,sizeof(aPNG))) {
		cerr << "PNG header match failure"<< endl;
		_errno = HEADER;
		return false;
	} else {
		debug(cout<<"Verified PNG header"<< endl;)
	}

	bool done=false;
	png_blk_type type; // Type of chunk
	uint32_t bytecount=0;	// Size of chunk data to read
	uint32_t bytesread=0; // Tally of all bytes read in iDAT
	uint32_t zbytesread=0; // Tally of how much data inflate()d
	uint32_t zavail=0; // Amount of space left in _image_buffer
	uint32_t crc;
	int palcnt; // Iterator for palette/trans handling
	png_pal_entry *pe; // Palette iterator
	png_bgtrns *pt; // Transparent palette iterator
	char cn[5]; // string version of header type for debug out
	cn[4]=0;
	char kwbuf[80];  // keyword buffer for *TXt chunks
	int ret;
	int trns_block_size=0;
	unsigned char *zptr=NULL;  // location where inflate() writes data in _image_buffer
	z_stream z; // zlib stream handle

	z.zalloc=Z_NULL;
	z.zfree=Z_NULL;
	z.opaque=Z_NULL;
	z.avail_in=0;
	z.next_in=Z_NULL;

	ret = inflateInit(&z);
	if (ret != Z_OK) {
		cerr<<"Error initializing zlib inflate!"<<endl;
		_errno = ZLIB;
		return false;
	}

	do {
		in.read(_buffer,sizeof(png_chunk));

		if (!in) {
			cerr << "error reading file"<<endl;
			_errno = READ;
			return false;
		}

		_chunk->len = bswap32(_chunk->len);

		memcpy(cn,&_chunk->type,4);

		debug(cout<<"chunk type "<<cn<<" len: "<<_chunk->len<<endl;)

		type = png_block_name(_chunk);

		crc = crc32(0L, Z_NULL, 0);  // courtesy of zlib.h
		crc = (uint32_t) crc32(crc, (const unsigned char *)_buffer+sizeof(_chunk->len),sizeof(_chunk->type)); // CRC of chunk header, not including len

		if (type != eIDAT) {
			if (_chunk->len > BUFSIZE) {
				cerr << "chunk len " << _chunk->len << " exceeds " << BUFSIZE<< endl;
				_errno = CHUNK_SIZE;
				return false;
			}
			in.read(_buffer+sizeof(png_chunk),_chunk->len);
			crc = (uint32_t) crc32(crc, (const unsigned char *)_buffer+sizeof(png_chunk), _chunk->len); // CRC of chunk data, not including chunk header
		}

		switch(type) {
			case eIHDR:
				_width=bswap32(IHDR->width);
				_height=bswap32(IHDR->height);
				_depth=IHDR->depth;
				_colors=(col_type)IHDR->ctype;
				_compress=IHDR->compress;
				_interlace=IHDR->interlace;

				switch (_colors) {
					case GRAY:
					case INDEXED:
						_bpp=_depth < 8 ? 1 : _depth/8;
						break;
					case RGB:
						_bpp=_depth/8 * 3;
						break;
					case GRAYA:
					case RGBA:
						_bpp=_depth/8 * 4;
						break;
				}

				bytesread=0;

				if (allocate_img_buffer() == false) {
					cerr << "Error allocating compress buffer"<<endl;
					(void)inflateEnd(&z);
					_errno = BADALLOC;
					return false;
				}

				zavail=_png_buf_size;
				zptr=_image_buffer;

				debug(cout<<"Image ("<<_width<<","<<_height<<") depth "<<(int)_depth<<" color_type "<<(int)_colors<<" compress "<<(int)_compress<<" interlace "<<(int)_interlace<< endl;)
			break;
			case ePLTE:
				_pal_size=_chunk->len/3;

				debug(cout<<"Palette of "<<_pal_size<<" entries"<< endl;)

				_pal=new png_pal_entry[_pal_size];

				if (!_pal) {
					cerr<<"Failed to allocate "<<palcnt<<" palette entries"<< endl;
					(void)inflateEnd(&z);
					_errno = BADALLOC;
					return false;
				}

				pe=&PLTE->pal;
				for(palcnt=0;palcnt<_pal_size;palcnt++) {
					memcpy(&_pal[palcnt],pe,sizeof(png_pal_entry));
					pe++;
					debug(cout<<palcnt<<": r "<<(int)_pal[palcnt].r<<" g "<<(int)_pal[palcnt].g<<" b "<<(int)_pal[palcnt].b<<endl;)
				}

			break;
			case eIDAT:
				debug(cout<<"IDAT chunk size "<<_chunk->len<< endl;)

				bytecount=_chunk->len;

				debug(cout<<"bytesread "<<bytesread<<endl;)

				while(bytecount) {
					in.read(_buffer,bytecount<BUFSIZE ? bytecount : BUFSIZE);

					crc = (uint32_t) crc32(crc, (const unsigned char *)_buffer,in.gcount());

					if (!in) {
						cerr<<"Error reading file"<<endl;
						(void)inflateEnd(&z);
						_errno = READ;
						return false;
					}

					bytesread+=in.gcount();
					bytecount-=in.gcount();
					debug(cout<<"read "<<in.gcount()<<endl;)

					z.avail_in=in.gcount();
					z.next_in=(unsigned char *)_buffer;
					debug(cout<<"Inflate "<<z.avail_in<<" bytes"<<endl;)

					z.avail_out=zavail;
					z.next_out=zptr;
					
					ret = inflate(&z, Z_SYNC_FLUSH);

					switch (ret) {
					case Z_NEED_DICT:
						ret = Z_DATA_ERROR;     
						// fall through
					case Z_MEM_ERROR:
					case Z_DATA_ERROR:
						(void)inflateEnd(&z);
						cerr<<"Inflate error "<<ret<<" "<<z.msg<<endl;
						_errno = ZLIB;
						return false;
					}

					debug(cout<<"Avail out after inflate pass "<<z.avail_out<<endl;)
					zbytesread = zavail-z.avail_out;
					zavail-=zbytesread;

					debug(cout<<"avail out "<<zavail<<endl;)
					debug(cout<<"Inflated "<<zbytesread<<" bytes"<<endl;)

					_uncompressed_len+=zbytesread;
					zptr+=zbytesread;

				}
				debug(cout<<"read "<<bytesread<<" bytes"<< endl;)
				break;
			case eIEND:
				debug(cout<<"IEND"<< endl;)

				(void)inflateEnd(&z);
				debug(cout<<"Uncompressed len "<<_uncompressed_len<<endl;)
				
				done=true;
				break;
			case etRNS:
				switch (_colors) {
					case GRAY:
						trns_block_size=sizeof(short);
						break;
					case INDEXED:
						trns_block_size=sizeof(char);
						break;
					case RGB:
						trns_block_size=sizeof(png_rgb);
						break;
					case GRAYA:
					case RGBA:
					continue;
				}

				_trns_size=_chunk->len/trns_block_size;
				debug(cout<<"tRNS chunk "<<type<<" of size "<<_chunk->len<< endl;)

				_trns=new png_bgtrns[_trns_size];

				if (!_trns) {
					cerr<<"Failed to allocate "<<_trns_size<<" palette entries"<< endl;
					(void)inflateEnd(&z);
					_errno = BADALLOC;
					return false;
				}

				for(palcnt=0;palcnt<_trns_size;palcnt++) {
					pt=&tRNS->alpha+(palcnt*trns_block_size);
					debug(cout<<"alpha b "<<(int)pt->ndx<< endl;)
					switch (_colors) {
						case GRAY:
							_trns[palcnt].gray=bswap16(pt->gray);
							break;
						case INDEXED:
							_trns[palcnt].ndx=pt->ndx;
							break;
						case GRAYA:
						case RGBA:
						case RGB:
							_trns[palcnt].rgb.r=bswap16(pt->rgb.r);
							_trns[palcnt].rgb.g=bswap16(pt->rgb.g);
							_trns[palcnt].rgb.b=bswap16(pt->rgb.b);
							break;
					}
					debug(cout<<"alpha "<<(int)_trns[palcnt].ndx<< endl;)
				}
				break;
			case ebKGD:
				debug(cout<<"bKGD chunk "<<type<<" of size "<<_chunk->len<<endl;)
				switch (_colors) {
						case GRAY:
							_bg.gray = bswap16(bKGD->bg.gray);
							break;
						case INDEXED:
							memcpy(&_bg,&bKGD->bg,_chunk->len);
							debug(cout<<"BG index is "<<(int)_bg.ndx<<endl;)
							break;
						case RGB:
						case GRAYA:
						case RGBA:
							_bg.rgb.r=bswap16(bKGD->bg.rgb.r);
							_bg.rgb.g=bswap16(bKGD->bg.rgb.g);
							_bg.rgb.b=bswap16(bKGD->bg.rgb.b);
							debug(cout<<"BG rgb is "<<(int)_bg.rgb.r<<" "<<(int)_bg.rgb.g<<" "<<(int)_bg.rgb.b<<endl;)
							break;
					}
				break;
			case egAMA:
			case ecHRM:
			case eiTXt:
			case etEXt:
				debug(cout<<"TEXT chunk "<<type<<" of size "<<_chunk->len<<endl;)
				strncpy(kwbuf,&(tEXt->text),sizeof(kwbuf)-1);
				kwbuf[sizeof(kwbuf)-1]='\0';
				ret=strlen(kwbuf);
				debug(cout<<"Keyword: "<<kwbuf<<endl;)
				bytecount=ret;
				while((&tEXt->text)[bytecount] == 0) bytecount++; // Eat extra \0 to get around noncompliant keyword delimiter
				strncpy(kwbuf,&(tEXt->text)+bytecount,_chunk->len-bytecount);
				kwbuf[_chunk->len-bytecount]=0;
				debug(cout<<"Text: "<<kwbuf<<endl;)
				break;
			case esRGB:
			case epHYs:
				debug(cout<<"pHYs chunk "<<type<<" of size "<<_chunk->len<<endl;)
				_ppu_x=bswap32(pHYs->ppu_x);
				_ppu_y=bswap32(pHYs->ppu_y);
				_unit=pHYs->unit;
				debug(cout<<"Phys dim x "<<_ppu_x<<" y "<<_ppu_y<<" unit "<<(int)_unit<<endl;)
				break;
			case esBIT:
				break;
			case etIME:
				debug(cout<<"tIME chunk "<<type<<" of size "<<_chunk->len<<endl;)
				break;
			case eUND:
			default:
				zavail=0;
				zptr=NULL;
				debug(cout<<"UNDefined chunk "<<type<<" of size "<<_chunk->len<< endl;)
				break;
		}
		in.read((char *)&_crc,sizeof(_crc));
		_crc=bswap32(_crc);
		if (_crc != crc) {
			//CRC error has occurred
			debug(cout<<"CRC error (read)"<<std::hex<<_crc<<" != (calc)"<<crc<<std::dec<<endl;)
			in.close();
			delete[] _buffer;
			_buffer=NULL;
			_errno = BADCRC;
			return false;
		}
	} while(!done);

	in.close();
	delete[] _buffer;
	_buffer=NULL;
	filter(); // run final filter step
	return true;
}

int png::paeth(unsigned char a, unsigned char b, unsigned char c)
{
		// a = left, b = above, c = upper left
		int p,pa,pb,pc;
		p = a + b - c;        // initial estimate
		pa = abs(p - a);      // distances to a, b, c
		pb = abs(p - b);
		pc = abs(p - c);
		// return nearest of a,b,c,
		// breaking ties in order a,b,c.
		if (pa <= pb && pa <= pc) return a;
		else if (pb <= pc) return b;
		else return c;
}

// implements PNG filter decoding
void png::filter(void)
{
	int x,y,b;
	unsigned char p,q,r,s,t;
	unsigned char filt;
	unsigned char *prior=_image_buffer;

	debug(cout<<"Bpp "<<_bpp<<endl;)
	debug(printhex(_image_buffer);)

	b=_scanline_size+1; // the first byte of a scanline is the filter type byte, this must be figured into address calculation

	/* PNG filters use the following nomenclature and rules:
	 *
	 * bpp = bytes per pixel, minimum of 1.  G = 1, GG = 2, RGB = 3, RGBA = 4, RRGGBB = 6, RRGGBBAA = 8
	 *
	 * Prior(x-bpp)--+   +---Prior(x)
	 *               |   |
	 * Filter type-v v   v
	 * Scanline 0: F RGB RGB RGB RGB
	 * Scanline 1: F RGB RGB RGB RGB
	 *   Raw(x-bpp)--^   ^--Filt_name(x) aka Raw(x)
	 *
	 * For line 0, Prior(x) will always be zero
	 * For RGB values in the first triple, Raw(x-bpp) and Prior(x-bpp) will always be zero, since they would shift off the beginning of the buffer.
	 * RGB triples can be 8bit depth or 16bit depth, the algorithms work at the byte level and compare R to R, G to G, and B to B of adjacent pixels.
	 * Grayscale can be a raw 1,2,4,8, or 16bit value, the algorithms assume a minimum of 1 byte per pixel.
	 *
	 */
	x=0;
	for (y=0;y<_height;y++) {
		filt=_image_buffer[y*b]; // filter byte is first byte in scanline

		debug(cout<<std::dec<<"line "<<y<<" Filter type "<<(int)filt<<endl;)
		switch(filt) {
		case 0: //none
			break;
		case 1: //sub
			for (x=1;x<b;x++) { // skip filter byte
				q=x<_bpp+1? 0 :_image_buffer[(y*b)+x-_bpp];
				p=_image_buffer[(y*b)+x];
				r=p+q;
				_image_buffer[(y*b)+x]=p+q;
				debug(cout << "sub q "<<std::hex<<(int)q<<" p "<<(int)p<<" result "<<(int)r<<endl;)
			}
			break;
		case 2: //up
			for (x=1;x<b;x++) { // skip filter byte
				q=_image_buffer[(y*b)+x];
				p= y==0 ? 0 : prior[x];
				r=p+q;
				_image_buffer[(y*b)+x]=p+q;
				debug(cout << "up q "<<std::hex<<(int)q<<" p "<<(int)p<<" result "<<(int)r<<endl;)
			}
			break;
		case 3: //average
			for (x=1;x<b;x++) { // skip filter byte
				p=_image_buffer[(y*b)+x];
				q=y==0 ? 0 : prior[x];
				r=x<_bpp+1? 0 :_image_buffer[(y*b)+x-_bpp];
				s=p+(r+q)/2;
				_image_buffer[(y*b)+x]=s;
				debug(cout << "avg p "<<std::hex<<(int)p<<" prior(x) "<<(int)q<<" raw(x-bpp) "<<(int)r<<" result "<<(int)s<<endl;)
			}
			break;
		case 4: //Paeth
			for (x=1;x<b;x++) { // skip filter byte
				q=x<_bpp+1 ? 0 : _image_buffer[(y*b)+x-_bpp]; // left Raw(x-bpp)
				r=y==0 ? 0 :prior[x]; // upper prior(x)
				s=x<_bpp+1 ? 0 : y==0 ? 0 : prior[x-_bpp]; // upper left  prior(x-bpp)
				p=_image_buffer[(y*b)+x]; // paeth(x)
				t=(unsigned char)paeth(q,r,s);
				_image_buffer[(y*b)+x]=p+t;
				debug(cout << "paeth Raw(x-bpp) "<<std::hex<<(int)q<<" prior(x) "<<(int)r<<" prior(x-bpp) "<<(int)s<<" raw(x) "<<(int)p<<" paeth(q,r,s) "<<(int)t<<" result "<<(int)(_image_buffer[(y*b)+x])<<endl;)
			}
			break;
		}
		debug(cout<<std::dec<<"y "<<y<<" b "<<b<<endl;)
		prior=&_image_buffer[y*b];
	}
}

bool png::convert(image& img)
{
	int x,y,i,index;
	unsigned char *pemap;
	unsigned char p,shiftbits;

	palette::pal_t ip;
	palette::pal_t *pixel;
	palette::pala_t *pixela;
	short gray_pixel;

	if (!img.size(_width,_height)) {
		return false;
	}

	debug(printhex(_image_buffer);)

	bool small_image=(_width*_height<512*512) ? true : false;  // a bit of optimization to reduce the cost of quantizing colors of RGB images, the threshold tradeoff is 512x512

	if (small_image) shiftbits=0; // We optimize the palette lookup by truncating 24bpp images to 18bbp to match VGA color space, no need to optimize if we have a small image
	else shiftbits=2;

	switch(_colors) {
		case GRAYA:
		case GRAY: {
			debug(cout<<"Building pemap for "<<(int)_depth<<" bpp"<<endl;)
			_pal_size=img.palette_size();
			pemap = new unsigned char[1<<_depth]; // Grayscale 1,2,4,8,16 bpp
			for (int x=0;x<(1<<_depth);x++) {
				// decimate 16bpp images, we can't show 64k shades of gray anyway
				ip.r=_depth < 16 ? x : x>>8;
				ip.g=_depth < 16 ? x : x>>8;
				ip.b=_depth < 16 ? x : x>>8;
				index = x;
				pemap[index]=img.findnearestpalentry(&ip);
//						cout <<"Index ["<<index<<"] = "<<(int)pemap[index]<<endl;
			}

			debug(cout<<"Built pemap "<<(1<<_depth)<<endl;)

			img.setbg(pemap[_bg.gray]);

			debug(cout<<"Set "<<(int)_bg.gray<<" as bg"<<endl;)
			i=0;
			for (y=0;y<_height;y++) {
				for(x=0;x<_width;) {
					switch (_depth) {
					case 1:
						p=_image_buffer[y*(_scanline_size+1)+1+(x/8)];
						img._buffer[i+0]=pemap[(p>>8)];
						img._buffer[i+1]=pemap[(p>>7&0x1)];
						img._buffer[i+2]=pemap[(p>>6&0x1)];
						img._buffer[i+3]=pemap[(p>>5&0x1)];
						img._buffer[i+4]=pemap[(p>>4&0x1)];
						img._buffer[i+5]=pemap[(p>>3&0x1)];
						img._buffer[i+6]=pemap[(p>>2&0x1)];
						img._buffer[i+7]=pemap[(p&0x1)];
						i+=8; // oct increment output
						x+=8;
						break;
					case 2:
						p=_image_buffer[y*(_scanline_size+1)+1+(x/4)];
						img._buffer[i]=pemap[(p>>6)];
						img._buffer[i+1]=pemap[(p>>4&0x3)];
						img._buffer[i+2]=pemap[(p>>2&0x3)];
						img._buffer[i+3]=pemap[(p&0x3)];
						i+=4; // quad increment output
						x+=4;
						break;
					case 4:
						p=_image_buffer[y*(_scanline_size+1)+1+(x/2)];
						debug(printf("pel: %02x\n",p);)
						img._buffer[i]=pemap[(p>>4)];
						img._buffer[i+1]=pemap[(p&0xF)];
						i+=2; // double increment output
						x+=2;
						break;
					case 8:
						p=_image_buffer[y*(_scanline_size+1)+1+x];
						debug(printf("pel: %02x\n",p);)
						img._buffer[i]=pemap[p];
						x++;
						i++;
						break;
					case 16:
						gray_pixel=bswap16(_image_buffer[y*(_scanline_size+1)+1+(x*sizeof(short))]);
						debug(printf("pel: %04x\n",gray_pixel);)
						img._buffer[i]=pemap[gray_pixel];
						x++;
						i++;
						break;
					}
				}
			}

			delete[] pemap;
			break;
		}
		case RGBA: {
			_pal_size=img.palette_size();

			if (!small_image) {
				pemap = new unsigned char[64*64*64]; // VGA is 6+6+6 so this is a hack to convert truecolor to the VGA 18 bit color space
				memset(pemap,255,64*64*64);
				/*
				for (unsigned char x=0;x<64;x++) {
					for (unsigned char y=0;y<64;y++) {
						for (unsigned char z=0;z<64;z++) {
							ip.r=x<<2;
							ip.g=y<<2;
							ip.b=z<<2;
							index = x*64*64+y*64+z;
							pemap[index]=img.findnearestpalentry(&ip);
	//						cout <<"Index ["<<index<<"] = "<<(int)pemap[index]<<endl;
						}
					}
				}
				*/

				debug(cout<<"Built pemap 64x64x64"<<endl;)
				ip.r=(unsigned char)_bg.rgb.r;
				ip.g=(unsigned char)_bg.rgb.g;
				ip.b=(unsigned char)_bg.rgb.b;
				img.setbg(img.findnearestpalentry(&ip));
//				img.setbg(pemap[((_bg.rgb.r>>2)*64*64)+((_bg.rgb.g>>2)*64)+(_bg.rgb.b>>2)]);
				debug(cout<<"BG small pal entry: "<<(int)img.getbg()<<endl;)
			} else {
				ip.r=(unsigned char)_bg.rgb.r;
				ip.g=(unsigned char)_bg.rgb.g;
				ip.b=(unsigned char)_bg.rgb.b;
				img.setbg(img.findnearestpalentry(&ip));
				debug(cout<<"BG pal entry: "<<(int)img.getbg()<<endl;)
			}

			switch (_depth) {
				case 8:
					i=0;
					for (y=0;y<_height;y++) {
						for(x=0;x<_width;x++) {
							pixela=(palette::pala_t *)&_image_buffer[y*(_scanline_size+1)+1+(x*sizeof(palette::pala_t))];
							debug(printf("r: %02x g: %02x b: %02x a: %02x\n",pixela->r,pixela->g,pixela->b,pixela->a);)
							/*
							pixela->r=pixela->r>>shiftbits;
							pixela->g=pixela->g>>shiftbits;
							pixela->b=pixela->b>>shiftbits;
							*/
							pixel=(palette::pal_t *)pixela;
//							index=(pixel->r>>shiftbits)*64*64+(pixel->g>>shiftbits)*64+(pixel->b>>shiftbits);
							img._buffer[i]=img.findnearestpalentry(pixel);
							/*
							if (small_image) {
								img._buffer[i]=img.findnearestpalentry(pixel);
							} else {
								if (pemap[index] == 255) {
									uncached++;
									pemap[index]=img.findnearestpalentry(pixel);
								} else {
									cached++;
								}
								img._buffer[i]=pemap[index];
							}
							*/
							i++;
						}
					}
					break;
				case 16:
					break;
			}

//			if (!small_image) delete[] pemap;
			break;
		}
		case RGB: {
			_pal_size=img.palette_size();

			if (!small_image) {
				pemap = new unsigned char[64*64*64]; // VGA is 6+6+6 so this is a hack to convert truecolor to the VGA 18 bit color space
				for (unsigned char x=0;x<64;x++) {
					for (unsigned char y=0;y<64;y++) {
						for (unsigned char z=0;z<64;z++) {
							ip.r=x<<2;
							ip.g=y<<2;
							ip.b=z<<2;
							index = x*64*64+y*64+z;
							pemap[index]=img.findnearestpalentry(&ip);
	//						cout <<"Index ["<<index<<"] = "<<(int)pemap[index]<<endl;
						}
					}
				}

				debug(cout<<"Built pemap 64x64x64"<<endl;)

				img.setbg(pemap[((_bg.rgb.r>>2)*64*64)+((_bg.rgb.g>>2)*64)+(_bg.rgb.b>>2)]);
				debug(cout<<"BG small pal entry: "<<(int)img.getbg()<<endl;)
			} else {
				ip.r=(unsigned char)_bg.rgb.r;
				ip.g=(unsigned char)_bg.rgb.g;
				ip.b=(unsigned char)_bg.rgb.b;
				debug(cout<<"Raw bg rgb "<<(int)ip.r<<" "<<(int)ip.g<<" "<<(int)ip.b<<endl;)
				img.setbg(img.findnearestpalentry(&ip));
				debug(printf("raw pal %02x %02x %02x\n",(img.getpalette())[img.getbg()].r,(img.getpalette())[img.getbg()].g,(img.getpalette())[img.getbg()].b);)
				debug(cout<<"BG pal entry: "<<(int)img.getbg()<<endl;)
			}

			switch (_depth) {
				case 8:
					debug(cout<<"Shiftbits: "<<shiftbits<<endl;)
					i=0;
					for (y=0;y<_height;y++) {
						for(x=0;x<_width;x++) {
							pixel=(palette::pal_t *)&_image_buffer[y*(_scanline_size+1)+1+(x*sizeof(palette::pal_t))];
							debug(printf("r: %02x g: %02x b: %02x =>",pixel->r,pixel->g,pixel->b);)
							//debug(printf("pel: %02x\n",p);)
							pixel->r=pixel->r>>shiftbits;
							pixel->g=pixel->g>>shiftbits;
							pixel->b=pixel->b>>shiftbits;
							debug(printf(" r: %02x g: %02x b: %02x => ",pixel->r,pixel->g,pixel->b);)
							index=pixel->r*64*64+pixel->g*64+pixel->b;
							img._buffer[i]=small_image ? img.findnearestpalentry(pixel) : pemap[index];
							debug(printf("%d\n",(int)img._buffer[i]);)
							debug(printf("raw pel %02x %02x %02x\n",(img.getpalette())[img._buffer[i]].r,(img.getpalette())[img._buffer[i]].g,(img.getpalette())[img._buffer[i]].b);)
							i++;
						}
					}
					std::cout<<"uncached: "<<img.miss<<" cached: "<<img.hit<<" hits/misses: "<<(float)img.hit/img.miss<<std::endl;
					break;
				case 16:
					break;
			}

//			if (!small_image) delete[] pemap;
			break;
		}
		case INDEXED: {
			pemap = new unsigned char[_pal_size];
			for (i=0; i<_pal_size; i++) {
				ip.r=_pal[i].r;
				ip.g=_pal[i].g;
				ip.b=_pal[i].b;
				pemap[i]=img.findnearestpalentry(&ip);
				debug(printf("pemap[%d]=%d %02x %d %d %d\n",i,pemap[i],pemap[i],_pal[i].r,_pal[i].g,_pal[i].b);)
			}

			switch (_depth) {
				case 1:
					break;
				case 2:
					for (y=0;y<img.height();y++) {
						i=0;
						for(x=0;x<img.width();x++) {
							p=_image_buffer[y*(_scanline_size+1)+1+i];
							debug(printf("pel: i %d %02x\n",i,p);)
							img._buffer[y*img.width()+x]=pemap[( p >> ((3-(x&3))*2)) &0x03];
							debug(printf("img[%02d][%02d]: %02x\n",y,x,img._buffer[y*img.width()+x]);)
							if ((x+1)%4==0) i++;
						}
					}
					break;
				case 4:
					i=0;
					for (y=0;y<_height;y++) {
						for(x=0;x<_width/2;x++) {
							p=_image_buffer[y*(_scanline_size+1)+1+x];
							debug(printf("pel: %02x\n",p);)
							img._buffer[i]=pemap[(p&0xF0)>>4];
							img._buffer[i+1]=pemap[p&0x0F];
							debug(printf("buf[h]=%02x buf[l]=%02x\n",img._buffer[i],img._buffer[i+1]);)
							i+=2;
						}
					}
					break;
				case 8:
					i=0;
					for (y=0;y<_height;y++) {
						for(x=0;x<_width;x++) {
							p=_image_buffer[y*(_scanline_size+1)+1+x];
							//debug(printf("pel: %02x\n",p);)
							img._buffer[i]=pemap[p];
							//debug(printf("buf[h]=%02x buf[l]=%02x\n",img._buffer[i],img._buffer[i+1]);)
							i++;
						}
					}
					break;
			}

			img.setbg(pemap[_bg.ndx]);
//			delete[] pemap;
			break;
		}
	}

	return true;
}

#ifdef TEST
int main(int argc, char *argv[])
{
	png p;
	image i;

	i.setpalette(palette::VGA_PAL);
	cout<<"Loading..."<<argv[1]<<endl;
	if (!p.load(argv[1])) {
		cout<<"Error loading PNG: "<<p.errormsg()<<endl;
		return 1;
	}
	cout<<"Converting..."<<endl;
	p.convert(i);

	debug(i.printhex();)

	return 0;
}
#endif
