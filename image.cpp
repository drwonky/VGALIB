#include <math.h>
#include <stdio.h>
#include "TYPES.H"
#include "MEMORY.H"
#include "IMAGE.H"
#include "sincos.h"
#include <string.h>

image::image(const image& img)
{
	w=img.w;
	h=img.h;
	bg=img.bg;
	allocate();
	memory::fast_memcpy(buffer,img.buffer,w*h);
	copypalette(img.palette());
}

image& image::operator = (const image& img)
{
	w=img.w;
	h=img.h;
	bg=img.bg;
	if (buffer) delete[] buffer;
	allocate();
	memory::fast_memcpy(buffer,img.buffer,w*h);
	copypalette(img.palette());

	return *this;
}

image::~image()
{
	this->free();
}

void image::free(void)
{
	if (buffer) delete[] buffer;
	if (mpalette) delete[] mpalette;
}

image::image(void)
{
	w=0;
	h=0;
	bg=0;
	buffer=NULL;
	mpalette=NULL;
}

image::image(unsigned int width, unsigned int height)
{
	bg=0;
	buffer=NULL;
	mpalette=NULL;
	size(width,height);
}

image::image(unsigned int width, unsigned int height, unsigned char background)
{
	bg=background;
	buffer=NULL;
	mpalette=NULL;
	size(width,height);
}

void image::copypalette(img_pal *p)
{
	memcpy(mpalette,p,sizeof(img_pal)*palette_size);
}

bool image::allocate(void)
{
	buffer=new unsigned far char [w*h];
	mpalette=new img_pal[palette_size];

	if(buffer) clear();
	if(mpalette) copypalette((img_pal *)vga_pal);

	return (bool)(buffer != NULL && mpalette != NULL);
}

bool image::size(unsigned int width, unsigned int height)
{
	if (buffer) delete[] buffer;
	w=width;
	h=height;
	return allocate();
}

int image::mappalentry(img_pal *p)
{
	int i;

	for (i=0; i<palette_size; i++) 
		if (mpalette[i].r == p->r &&
			mpalette[i].g == p->g &&
			mpalette[i].b == p->b) {
				//printf("pal entry %d %d %d found at %d %02x\n",mpalette[i].r,mpalette[i].g,mpalette[i].b,i,i);
				return i;
			}

	return palette_size-1;
}

img_pal image::getpalentry(int i)
{
	return (img_pal)mpalette[i];
}

void image::setbg(unsigned char background)
{
	bg=background;
}

unsigned char image::getbg(void)
{
	return bg;
}

unsigned int image::width(void)
{
	return w;
}

unsigned int image::height(void)
{
	return h;
}

void image::clear(unsigned char color)
{
	unsigned int p=w*h;

	while(p--) buffer[p]=color;
}

void image::clear(void)
{
	clear(0);
}

void image::setpixel(unsigned int x, unsigned int y)
{
	setpixel(x,y,bg);
}

void image::setpixel(unsigned int x, unsigned int y, unsigned char color)
{
	buffer[y*w+x]=color;
}

unsigned char image::getpixel(unsigned int x, unsigned int y)
{
	return buffer[y*w+x];
}

void image::drawbox(int x,int y,int width,int height,unsigned char color)
{
	int x1,y1;
	for(y1=y;y1<y+height;y1++)
	{
		for(x1=x;x1<x+width;x1++)
		{
		   setpixel(x1,y1,color);
		}
	}
}

void image::drawbox(int x, int y, int width, int height, unsigned char color, int filled)
{
	if (filled) {
		drawbox(x,y,w,h,color);
	} else {
		line(x,y,x+width,y,color);
		line(x,y,x,y+height,color);
		line(x,y+height,x+width,y+height,color);
		line(x+width,y,x+width,y+height,color);
	}
}

void image::line(int x1, int y1, int x2, int y2, unsigned char c)
{
		int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;

		dx=x2-x1;
		dy=y2-y1;
		dx1=abs(dx);
		dy1=abs(dy);
		px=2*dy1-dx1;
		py=2*dx1-dy1;

		if(dy1<=dx1)
		{
				if(dx>=0)
				{
						x=x1;
						y=y1;
						xe=x2;
				}
				else
				{
						x=x2;
						y=y2;
						xe=x1;
				}
				setpixel(x,y,c);
				for(i=0;x<xe;i++)
				{
						x=x+1;
						if(px<0)
						{
								px=px+2*dy1;
						}
						else
						{
								if((dx<0 && dy<0) || (dx>0 && dy>0))
								{
										y=y+1;
								}
								else
								{
										y=y-1;
								}
								px=px+2*(dy1-dx1);
						}
						setpixel(x,y,c);
				}
		}
		else
		{
				if(dy>=0)
				{
						x=x1;
						y=y1;
						ye=y2;
				}
				else
				{
						x=x2;
						y=y2;
						ye=y1;
				}
				setpixel(x,y,c);
				for(i=0;y<ye;i++)
				{
						y=y+1;
						if(py<=0)
						{
								py=py+2*dx1;
						}
						else
						{
								if((dx<0 && dy<0) || (dx>0 && dy>0))
								{
										x=x+1;
								}
								else
								{
										x=x-1;
								}
								py=py+2*(dx1-dy1);
						}
						setpixel(x,y,c);
				}
		}
}

void image::copyto(unsigned int x, unsigned int y, unsigned int x1, unsigned int y1, unsigned int width, unsigned int height)
{
	unsigned int src,rem,dest,cnt;
	unsigned char far *tmp;

	if (x1>w-1 || y1>h-1) return;

	width=x1+width > x ? x-x1 : width;
	height=y1+height > y ? y-y1 : height;
	src=y*w+x;
	dest=y1*w+x1;
	cnt=height;
	tmp=new unsigned char[width*height];
	rem=0;
	while(cnt) {
		memory::fast_memcpy(&tmp[rem],&buffer[src],width);
		rem+=width;
		src+=width;
		cnt--;
	}
	cnt=height;
	rem=0;
	while(cnt) {
		memory::fast_memcpy(&buffer[dest],&tmp[rem],width);
		dest+=w;
		rem+=width;
		cnt--;
	}
	delete[] tmp;
}

void image::copyto(image far *src, image far *dest, unsigned int sx, unsigned int sy, unsigned int dx, unsigned int dy, unsigned int width, unsigned int height)
{
	unsigned int rem,dcnt,cnt,dw,iw;

	if (dx>dest->width()-1 || dy>dest->height()-1) return;

	iw=src->width();
	width=dx+width > dest->width() ? dest->width()-dx : width;
	height=dy+height > dest->height() ? dest->height()-dy : height;
	rem=sy*src->width()+sx;
	dcnt=dy*dest->width()+dx;
	cnt=height;
	rem=0;
	dw=dest->width();
	while(cnt) {
		memory::fast_memcpy(&dest->buffer[dcnt],&src->buffer[rem],width);
		dcnt+=dw;
		rem+=iw;
		cnt--;
	}
}

/*
void image::rotate(image far *dest, int angle)
{
	int hwidth = w / 2;
	int hheight = h / 2;
	double sinma = sin(-angle*M_PI/180);
	double cosma = cos(-angle*M_PI/180);
	int xt,yt,xs,ys;

	for(int x = 0; x < w; x++) {
		xt = x - hwidth;
		for(int y = 0; y < h; y++) {
			yt = y - hheight;

			xs = (int)floor((cosma * xt - sinma * yt) + hwidth);
			ys = (int)floor((sinma * xt + cosma * yt) + hheight);

			if(xs >= 0 && xs < w && ys >= 0 && ys < h) {
				dest->setpixel(x,y,getpixel(xs,ys));
			} else {
				dest->setpixel(x,y);
			}
		}
	}
}
*/

void image::rotate(image& dest, int angle)
{
	int hwidth = w / 2;
	int hheight = h / 2;
	long sinma = sindeg[angle];
	long cosma = cosdeg[angle];
	long xt,yt,xs,ys;
	unsigned int x,y;

	for(x = 0; x < w; x++) {
		xt = x - hwidth;
		for(y = 0; y < h; y++) {
			yt = y - hheight;

			xs =  cosma * xt;
			xs -= sinma * yt;
			xs >>= 14;
			xs += hwidth;
			ys =  sinma * xt;
			ys += cosma * yt;
			ys >>= 14;
			ys += hheight;
			if(xs >= 0 && xs < w && ys >= 0 && ys < h) {
				dest.buffer[y*w+x]=buffer[ys*w+xs];
//				dest.setpixel(x,y,getpixel(xs,ys));
			} else {
//				dest.setpixel(x,y);
				dest.buffer[y*w+x]=bg;
			}
		}
	}
}

void image::rotate(int angle)
{
	image tmp = *this;

	tmp.rotate(*this,angle);

}

void image::scale(image& dest, int width, int height)
{
	int x,y;

	dest.bg=bg;
	dest.copypalette(mpalette);

	for (y=0;y<height;y++) {
		for (x=0;x<width;x++) {
			dest.buffer[y*width+x]=buffer[(y*h/height)*w+(x*w/width)];
		}
	}
}

image& image::scale(int width, int height)
{
	image *ret = new image(width,height);

	scale(*ret,width,height);

	ret->copypalette(mpalette);
	ret->bg=bg;

	return *ret;
}

void image::scale(image& img)
{
	unsigned int x,y;
	int iw=img.width();
	int ih=img.height();
	bg=img.getbg();
	copypalette(img.palette());

	for (y=0;y<h;y++) {
		for (x=0;x<w;x++) {
			setpixel(x,y,img.getpixel(x*iw/w,y*ih/h));
		}
	}
}

void image::printhex(void)
{
    unsigned int i,j,b;

    b=w;

	printf("width: %d height: %d\n",w,h);
    printf("    ");
    for (i=0;i<b;i++) {
        printf("%2d ",i);
    }
    printf("\n");
    for (j=0;j<h;j++) {
        printf("%2d: ",j);
        for (i=0;i<b;i++) {
            printf("%02x ",buffer[j*w+i]);
        }
        printf("\n");
    }
}
