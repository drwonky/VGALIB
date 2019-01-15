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

#define BUFSIZE 8192

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

}

png::~png(void)
{
	this->free();
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
			return _width*_depth/8;
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
	_buffer = new char[BUFSIZE];

	if (!_buffer) {
		cerr << "Error allocating file buffer"<<endl;
		return false;
	}

	ifstream in;

	debug(cout<<"Loading "<<file<<endl;)
	in.open(file,fstream::binary);

	if (!in) {
		cerr << "error opening" << file << endl;
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
		return false;
	} else {
		debug(cout<<"Verified PNG header"<< endl;)
	}

	bool done=false;
	png_blk_type type; // Type of chunk
	uint32_t bytecount;	// Size of chunk data to read
	uint32_t bytesread=0; // Tally of all bytes read in iDAT
	uint32_t zbytesread; // Tally of how much data inflate()d
	uint32_t zavail; // Amount of space left in _image_buffer
	int palcnt; // Iterator for palette/trans handling
	png_pal_entry *pe; // Palette iterator
	png_bgtrns *pt; // Transparent palette iterator
	char cn[5]; // string version of header type for debug out
	cn[4]=0;
	char kwbuf[80];  // keyword buffer for *TXt chunks
	int ret;
	int trns_block_size=0;
	unsigned char *zptr;  // location where inflate() writes data in _image_buffer
	z_stream z; // zlib stream handle

	z.zalloc=Z_NULL;
	z.zfree=Z_NULL;
	z.opaque=Z_NULL;
	z.avail_in=0;
	z.next_in=Z_NULL;

	ret = inflateInit(&z);
	if (ret != Z_OK) {
		cerr<<"Error initializing zlib inflate!"<<endl;
		return false;
	}

	do {
		in.read(_buffer,sizeof(png_chunk));

		if (!in) {
			cerr << "error reading file"<<endl;
			return false;
		}

		_chunk->len = bswap32(_chunk->len);

		type = png_block_name(_chunk);

		if (type != eIDAT) {
			if (_chunk->len > BUFSIZE) {
				cerr << "chunk len " << _chunk->len << " exceeds " << BUFSIZE<< endl;
				return false;
			}
			in.read(_buffer+sizeof(png_chunk),_chunk->len);
		}

		memcpy(cn,&_chunk->type,4);
		
		debug(cout<<"chunk type "<<cn<<endl;)
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
						_bpp=_depth < 8 ? 1 : _depth/8;
						break;
					case INDEXED:
						_bpp=_depth < 8 ? 1 : _depth/8;
						_bpp*=3;
						break;
					case RGB:
						_bpp=_depth/8 * 3;
						break;
					case GRAYA:
					case RGBA:
						_bpp=_depth/8 * 4;
					continue;
				}

				bytesread=0;

				if (allocate_img_buffer() == false) {
					cerr << "Error allocating compress buffer"<<endl;
					(void)inflateEnd(&z);
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

					if (!in) {
						cerr<<"Error reading file"<<endl;
						(void)inflateEnd(&z);
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
					case Z_MEM_ERROR:
					case Z_DATA_ERROR:
						(void)inflateEnd(&z);
						cerr<<"Inflate error "<<ret<<" "<<z.msg<<endl;
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
				//printhex(_image_buffer);
				
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
						case RGB:
							_trns[palcnt].rgb.r=bswap16(pt->rgb.r);
							_trns[palcnt].rgb.g=bswap16(pt->rgb.g);
							_trns[palcnt].rgb.b=bswap16(pt->rgb.b);
							break;
						case GRAYA:
						case RGBA:
							break;
					}
					debug(cout<<"alpha "<<(int)_trns[palcnt].ndx<< endl;)
				}
				break;
			case ebKGD:
				debug(cout<<"bKGD chunk "<<type<<" of size "<<_chunk->len<<endl;)
				switch (_colors) {
						case GRAY:
							break;
						case INDEXED:
							memcpy(&_bg,&bKGD->bg,_chunk->len);
							debug(cout<<"BG index is "<<(int)_bg.ndx<<endl;)
							break;
						case RGB:
							_bg.rgb.r=bswap16(bKGD->bg.rgb.r);
							_bg.rgb.g=bswap16(bKGD->bg.rgb.g);
							_bg.rgb.b=bswap16(bKGD->bg.rgb.b);
							debug(cout<<"BG rgb is "<<(int)_bg.rgb.r<<" "<<(int)_bg.rgb.g<<" "<<(int)_bg.rgb.b<<endl;)
							break;
						case GRAYA:
						case RGBA:
							break;
					}
				break;
			case egAMA:
			case ecHRM:
			case eiTXt:
			case etEXt:
				debug(cout<<"TEXT chunk "<<type<<" of size "<<_chunk->len<<endl;)
				strncpy(kwbuf,&(tEXt->text),80);
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
				debug(cout<<"UNDefined chunk "<<type<<" of size "<<_chunk->len<< endl;)
				break;
		}
		in.read((char *)&_crc,sizeof(_crc));
	} while(!done);

	in.close();
	delete[] _buffer;
	_buffer=NULL;
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

void png::filter(void)
{
	int x,y,b,ybytes;
	unsigned char p,q,r,s,t;
	unsigned char filt;
	unsigned char *prior;

	debug(cout<<"Bpp "<<_bpp<<endl;)
	printhex(_image_buffer);

	ybytes=_uncompressed_len/_height;

	b=bytes_per_scanline()+1;

	x=0;
	for (y=0;y<_height;y++) {
		filt=_image_buffer[y*ybytes]; // filter byte is first byte in scanline

		debug(cout<<std::dec<<"line "<<y<<" Filter type "<<(int)filt<<endl;)
		switch(filt) {
		case 0: //none
			continue;
		case 1: //sub
			for (x=1;x<b;x++) { // skip filter byte
				q=x<_bpp+1? 0 :_image_buffer[(y*ybytes)+x-_bpp];
				p=_image_buffer[(y*ybytes)+x];
				r=p+q;
				_image_buffer[(y*ybytes)+x]=p+q;
				cout << "sub q "<<std::hex<<(int)q<<" p "<<(int)p<<" result "<<(int)r<<endl;
			}
			break;
		case 2: //up
			for (x=1;x<b;x++) { // skip filter byte
				q=_image_buffer[(y*ybytes)+x];
				p= y==0 ? 0 : prior[x];
				r=p+q;
				_image_buffer[(y*ybytes)+x]=p+q;
				cout << "up q "<<std::hex<<(int)q<<" p "<<(int)p<<" result "<<(int)r<<endl;
			}
			break;
		case 3: //average
			for (x=1;x<b;x++) { // skip filter byte
				p=_image_buffer[(y*ybytes)+x];
				q=y==0 ? 0 : prior[x];
				r=x<_bpp+1? 0 :_image_buffer[(y*ybytes)+x-_bpp];
				s=p+(r+q)/2;
				_image_buffer[(y*ybytes)+x]=s;
				cout << "avg p "<<std::hex<<(int)p<<" prior(x) "<<(int)q<<" raw(x-bpp) "<<(int)r<<" result "<<(int)s<<endl;
			}
			break;
		case 4: //Paeth
			for (x=1;x<b;x++) { // skip filter byte
				q=x<_bpp+1 ? 0 : _image_buffer[(y*ybytes)+x-_bpp]; // left Raw(x-bpp)
				r=y==0 ? 0 :prior[x]; // upper prior(x)
				s=x<_bpp+1 ? 0 : y==0 ? 0 : prior[x-_bpp]; // upper left  prior(x-bpp)
				p=_image_buffer[(y*ybytes)+x]; // paeth(x)
				t=(unsigned char)paeth(q,r,s);
				_image_buffer[(y*ybytes)+x]=p+t;
				cout << "paeth Raw(x-bpp) "<<std::hex<<(int)q<<" prior(x) "<<(int)r<<" prior(x-bpp) "<<(int)s<<" raw(x) "<<(int)p<<" paeth(q,r,s) "<<(int)t<<" result "<<(int)(_image_buffer[(y*ybytes)+x])<<endl;
			}
			break;
		}
		prior=&_image_buffer[y*ybytes];
	}
}

bool png::convert2image(image& img)
{
	int x,y,i,index;
	unsigned char *pemap;
	unsigned char p;

	palette::pal_t ip;
	palette::pal_t *pixel,ppixel;

	if (!img.size(_width,_height))
		return false;

	printhex(_image_buffer);

	switch(_colors) {
		case GRAY:
			break;
		case RGB:
			_pal_size=img.palette_size();
			pemap = new unsigned char[64*64*64];
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

			img.setbg(pemap[_bg.rgb.r>>2*64*64+_bg.rgb.g>>2*64+_bg.rgb.b>>2]);

			switch (_depth) {
				case 8:
					i=0;
					for (y=0;y<_height;y++) {
						for(x=0;x<_width;x++) {
							pixel=(palette::pal_t *)&_image_buffer[y*(_scanline_size+1)+1+(x*sizeof(palette::pal_t))];
							//debug(printf("pel: %02x\n",p);)
							ppixel.r=pixel->r;
							ppixel.g=pixel->g;
							ppixel.b=pixel->b;
							pixel->r=pixel->r>>2;
							pixel->g=pixel->g>>2;
							pixel->b=pixel->b>>2;
							index=pixel->r*64*64+pixel->g*64+pixel->b;
							if (y==32) cout<<"r "<<(int)pixel->r<<"("<<(int)ppixel.r<<") g "<<(int)pixel->g<<"("<<(int)ppixel.g<<") b "<<(int)pixel->b<<"("<<(int)ppixel.b<<") Index ["<<index<<"] = "<<(int)pemap[index]<<endl;
							img.buffer[i]=pemap[index];
							//debug(printf("buf[h]=%02x buf[l]=%02x\n",img.buffer[i],img.buffer[i+1]);)
							i++;
						}
					}
					break;
				case 16:
					break;
			}

			delete[] pemap;
			break;
		case INDEXED:
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
					break;
				case 4:
					i=0;
					for (y=0;y<_height;y++) {
						for(x=0;x<_width/2;x++) {
							p=_image_buffer[y*(_scanline_size+1)+1+x];
							debug(printf("pel: %02x\n",p);)
							img.buffer[i]=pemap[(p&0xF0)>>4];
							img.buffer[i+1]=pemap[p&0x0F];
							debug(printf("buf[h]=%02x buf[l]=%02x\n",img.buffer[i],img.buffer[i+1]);)
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
							img.buffer[i]=pemap[p];
							//debug(printf("buf[h]=%02x buf[l]=%02x\n",img.buffer[i],img.buffer[i+1]);)
							i++;
						}
					}
					break;
			}

			img.setbg(pemap[_bg.ndx]);
			delete[] pemap;
			break;
		case RGBA:
			break;
		case GRAYA:
			break;
	}

	return true;
}

#ifdef TEST
int main(int argc, char *argv[])
{
	png p;
	image i;

	i.setpalette(palette::RGB_PAL);
	p.load(argv[1]);
	p.filter();
	p.convert2image(i);

	debug(i.printhex();)

	return 0;
}
#endif
