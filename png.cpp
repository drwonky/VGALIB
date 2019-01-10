#include <stdio.h>
#include <string.h>
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

	image_buffer=NULL;
	buffer=NULL;
	pal=NULL;
	trns=NULL;
	pal_size=0;
	trns_size=0;
	width=0;
	height=0;
	colors=GRAY;
	depth=0;
	interlace=0;
	compress=0;
	ppu_x=0;
	ppu_y=0;
	unit=0;
	scanline_size=0;
	len=0;
	crc=0;
	png_buf_size=0;
	uncompressed_len=0;

}

png::~png(void)
{
	this->free();
}

void png::free(void)
{
	if (image_buffer) { delete[] image_buffer; image_buffer=NULL; }
	if (pal) { delete[] pal; pal=NULL; }
	if (trns) { delete[] trns; trns=NULL; }
	if (buffer) { delete[] buffer; buffer=NULL; }
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

	b=bytes_per_scanline();

	printf("    ");
	for (i=0;i<b;i++) {
		printf("%2d ",i);
	}
	printf("\n");
	for (j=0;j<height;j++) {
		printf("%2d: ",j);
		for (i=0;i<b;i++) {
			printf("%02x ",buf[(j*uncompressed_len/height)+1+i]);
		}
		printf("\n");
	}
}

int png::bytes_per_scanline(void)
{
	switch(colors) {
		case GRAY:
		case INDEXED:
			return width*depth/8;
		case RGB:
			return width*depth*3/8;
		case GRAYA:
			return width*(depth/8+1);
		case RGBA:
			return width*(depth*3/8+1);
	}

	return 0;
}

bool png::allocate_img_buffer(void)
{
	png_buf_size=scanline_size=bytes_per_scanline();

	debug(cout<<"Calculated "<<png_buf_size<<" per line"<<endl;)
	png_buf_size++; //filter mode at begin of each scanline

	png_buf_size*=height;

	debug(cout<<"Allocated "<<png_buf_size<<" bytes for image buffer"<<endl;)

	image_buffer = new unsigned char[png_buf_size];

	if (image_buffer != NULL) return true;
	return false;
}

bool png::load(char *file)
{
	buffer = new char[BUFSIZE];

	if (!buffer) {
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
    len = in.tellg();
    in.seekg (0, in.beg);
	uncompressed_len=0;

	debug(cout<<"File length "<<len<<endl;)

	in.read(buffer,sizeof(png_signature));

	if (memcmp(buffer,aPNG,sizeof(aPNG))) {
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
	uint32_t zavail; // Amount of space left in image_buffer
	int palcnt; // Iterator for palette/trans handling
	png_pal_entry *pe; // Palette iterator
	png_bgtrns *pt; // Transparent palette iterator
	char cn[5]; // string version of header type for debug out
	cn[4]=0;
	char kwbuf[80];  // keyword buffer for *TXt chunks
	int ret;
	int trns_block_size=0;
	unsigned char *zptr;  // location where inflate() writes data in image_buffer
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
		in.read(buffer,sizeof(png_chunk));

		if (!in) {
			cerr << "error reading file"<<endl;
			return false;
		}

		chunk->len = bswap32(chunk->len);

		type = png_block_name(chunk);

		if (type != eIDAT) {
			if (chunk->len > BUFSIZE) {
				cerr << "chunk len " << chunk->len << " exceeds " << BUFSIZE<< endl;
				return false;
			}
			in.read(buffer+sizeof(png_chunk),chunk->len);
		}

		memcpy(cn,&chunk->type,4);
		
		debug(cout<<"chunk type "<<cn<<endl;)
		switch(type) {
			case eIHDR:
				width=bswap32(IHDR->width);
				height=bswap32(IHDR->height);
				depth=IHDR->depth;
				colors=(col_type)IHDR->ctype;
				compress=IHDR->compress;
				interlace=IHDR->interlace;

				bytesread=0;

				if (allocate_img_buffer() == false) {
					cerr << "Error allocating compress buffer"<<endl;
					(void)inflateEnd(&z);
					return false;
				}

				zavail=png_buf_size;
				zptr=image_buffer;

				debug(cout<<"Image ("<<width<<","<<height<<") depth "<<(int)depth<<" color_type "<<(int)colors<<" compress "<<(int)compress<<" interlace "<<(int)interlace<< endl;)
			break;
			case ePLTE:
				pal_size=chunk->len/3;

				debug(cout<<"Palette of "<<pal_size<<" entries"<< endl;)

				pal=new png_pal_entry[pal_size];

				if (!pal) {
					cerr<<"Failed to allocate "<<palcnt<<" palette entries"<< endl;
					(void)inflateEnd(&z);
					return false;
				}

				pe=&PLTE->pal;
				for(palcnt=0;palcnt<pal_size;palcnt++) {
					memcpy(&pal[palcnt],pe,sizeof(png_pal_entry));
					pe++;
					debug(cout<<palcnt<<": r "<<(int)pal[palcnt].red<<" g "<<(int)pal[palcnt].green<<" b "<<(int)pal[palcnt].blue<<endl;)
				}

			break;
			case eIDAT:
				debug(cout<<"IDAT chunk size "<<chunk->len<< endl;)

				bytecount=chunk->len;

				debug(cout<<"bytesread "<<bytesread<<endl;)

				while(bytecount) {
					in.read(buffer,bytecount<BUFSIZE ? bytecount : BUFSIZE);

					if (!in) {
						cerr<<"Error reading file"<<endl;
						(void)inflateEnd(&z);
						return false;
					}

					bytesread+=in.gcount();
					bytecount-=in.gcount();
					debug(cout<<"read "<<in.gcount()<<endl;)

					z.avail_in=in.gcount();
					z.next_in=(unsigned char *)buffer;
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

					uncompressed_len+=zbytesread;
					zptr+=zbytesread;

				}
				debug(cout<<"read "<<bytesread<<" bytes"<< endl;)
				break;
			case eIEND:
				debug(cout<<"IEND"<< endl;)

				(void)inflateEnd(&z);
				debug(cout<<"Uncompressed len "<<uncompressed_len<<endl;)
				//printhex(image_buffer);
				
				done=true;
				break;
			case etRNS:
				switch (colors) {
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

				trns_size=chunk->len/trns_block_size;
				debug(cout<<"tRNS chunk "<<type<<" of size "<<chunk->len<< endl;)

				trns=new png_bgtrns[trns_size];

				if (!trns) {
					cerr<<"Failed to allocate "<<trns_size<<" palette entries"<< endl;
					(void)inflateEnd(&z);
					return false;
				}

				for(palcnt=0;palcnt<trns_size;palcnt++) {
					pt=&tRNS->alpha+(palcnt*trns_block_size);
					debug(cout<<"alpha b "<<(int)pt->ndx<< endl;)
					switch (colors) {
						case GRAY:
							trns[palcnt].gray=bswap16(pt->gray);
							break;
						case INDEXED:
							trns[palcnt].ndx=pt->ndx;
							break;
						case RGB:
							trns[palcnt].rgb.r=bswap16(pt->rgb.r);
							trns[palcnt].rgb.g=bswap16(pt->rgb.g);
							trns[palcnt].rgb.b=bswap16(pt->rgb.b);
							break;
						case GRAYA:
						case RGBA:
							break;
					}
					debug(cout<<"alpha "<<(int)trns[palcnt].ndx<< endl;)
				}
				break;
			case ebKGD:
				debug(cout<<"bKGD chunk "<<type<<" of size "<<chunk->len<<endl;)
				memcpy(&bg,&bKGD->bg,chunk->len);
				debug(cout<<"BG index is "<<(int)bg.ndx<<endl;)
				break;
			case egAMA:
			case ecHRM:
			case eiTXt:
			case etEXt:
				debug(cout<<"TEXT chunk "<<type<<" of size "<<chunk->len<<endl;)
				strncpy(kwbuf,&(tEXt->text),80);
				ret=strlen(kwbuf);
				debug(cout<<"Keyword: "<<kwbuf<<endl;)
				bytecount=ret;
				while((&tEXt->text)[bytecount] == 0) bytecount++; // Eat extra \0 to get around noncompliant keyword delimiter
				strncpy(kwbuf,&(tEXt->text)+bytecount,chunk->len-bytecount);
				kwbuf[chunk->len-bytecount]=0;
				debug(cout<<"Text: "<<kwbuf<<endl;)
				break;
			case esRGB:
			case epHYs:
				debug(cout<<"pHYs chunk "<<type<<" of size "<<chunk->len<<endl;)
				ppu_x=bswap32(pHYs->ppu_x);
				ppu_y=bswap32(pHYs->ppu_y);
				unit=pHYs->unit;
				debug(cout<<"Phys dim x "<<ppu_x<<" y "<<ppu_y<<" unit "<<(int)unit<<endl;)
				break;
			case esBIT:
				break;
			case etIME:
				debug(cout<<"tIME chunk "<<type<<" of size "<<chunk->len<<endl;)
				break;
			case eUND:
			default:
				debug(cout<<"UNDefined chunk "<<type<<" of size "<<chunk->len<< endl;)
				break;
		}
		in.read((char *)&crc,sizeof(crc));
	} while(!done);

	in.close();
	delete[] buffer;
	buffer=NULL;
	return true;
}

bool png::convert2image(image& img)
{
	int x,y,i;
	unsigned char *pemap;
	unsigned char p;
	img_pal ip;

	if (!img.size(width,height))
		return false;

	//printhex(image_buffer);

	switch(colors) {
		case GRAY:
			break;
		case RGB:
			break;
		case INDEXED:
			pemap = new unsigned char[pal_size];
			for (i=0; i<pal_size; i++) {
				ip.r=pal[i].red;
				ip.g=pal[i].green;
				ip.b=pal[i].blue;
				pemap[i]=img.findnearestpalentry(&ip);
				debug(printf("pemap[%d]=%d %02x %d %d %d\n",i,pemap[i],pemap[i],pal[i].red,pal[i].green,pal[i].blue);)
			}

			switch (depth) {
				case 1:
					break;
				case 2:
					break;
				case 4:
					i=0;
					for (y=0;y<height;y++) {
						for(x=0;x<width/2;x++) {
							p=image_buffer[y*(scanline_size+1)+1+x];
							debug(printf("pel: %02x\n",p);)
							img.buffer[i]=pemap[(p&0xF0)>>4];
							img.buffer[i+1]=pemap[p&0x0F];
							debug(printf("buf[h]=%02x buf[l]=%02x\n",img.buffer[i],img.buffer[i+1]);)
							i+=2;
						}
					}
					break;
				case 8:
					break;
			}

			img.setbg(pemap[bg.ndx]);
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

	i.setpalette(CGA_PAL);
	p.load(argv[1]);
	p.convert2image(i);

	debug(i.printhex();)

	return 0;
}
#endif
