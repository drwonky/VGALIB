#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}

double H(rgb *a)
{
	double result;
	double r=a->r,g=a->g,b=a->b;

	result=((r-g)+(r-b))/2;
	result=result/(((r-g)*(r-g))+((r-b)*(g-b)));
	result=sqrt(result);
	result=acos(result);

	return result;
}

int cmp(const void *aa, const void *bb)
{
/*	int32_t R,G,B,dist;*/
	rgb *a,*b;
	double ad,bd;
	hsv ha,hb;

	a=(rgb *)aa;
	b=(rgb *)bb;
	
	ad=(a->r*a->r)*2+(a->g*a->g)*4+(a->b*a->b)*3;
	bd=(b->r*b->r)*2+(b->g*b->g)*4+(b->b*b->b)*3;

	ha=rgb2hsv(*a);
	hb=rgb2hsv(*b);

	ad=ha.h+ha.v;
	bd=hb.h+hb.v;

/*
	R=(int32_t)(a->r-b->r)*(int32_t)(a->r-b->r);
    G=(int32_t)(a->g-b->g)*(int32_t)(a->g-b->g);
    B=(int32_t)(a->b-b->b)*(int32_t)(a->b-b->b);
    dist=2*R+4*G+3*B;
*/

/*	if (ad<bd) return -1;
	else if (ad>bd) return 1;
	else return 0;*/

	if (ha.h<hb.h) 
		if (ha.s<hb.s)
			if (ha.v<hb.v) return -1;
			else if (ha.v>hb.v) return 1;
		else if (ha.s>hb.s)
			if (ha.v<hb.v) return -1;
			else if (ha.v>hb.v) return 1;
	else if (ha.h>hb.h)
		if (ha.s<hb.s)
			if (ha.v<hb.v) return -1;
			else if (ha.v>hb.v) return 1;
		else if (ha.s>hb.s)
			if (ha.v<hb.v) return -1;
			else if (ha.v>hb.v) return 1;
	else return 0;

/*	if (ha.h<hb.h) return -1;
	else if (ha.h>hb.h) return 1;
	else if (ha.s<hb.s) return -1;
	else if (ha.s>hb.s) return 1;
	else if (ha.v<hb.v) return -1;
	else if (ha.v>hb.v) return 1;
	else {
		return 0;
	}*/
	
}

static int col8 [] = {0,32,84,112,144,174,224,255};
static int col4 [] = {0,84,174,255};

int main(void)
{
	rgb colors[256]; 
	int r,g,b;
	int i=0;

//	for (i=0;i<256;i++)
//		printf ("{%d,%d,%d}%c\n",col4[(i>>6)&0x3],col8[(i>>3)&0x7],col8[i&7],i<255?',':' ');
		//printf ("{%d,%d,%d}%c\n",col8[(i>>5)&0x7],col4[(i>>3)&0x3],col8[i&7],i<255?',':' ');
		//printf ("{%d,%d,%d}%c\n",col8[(i>>5)&0x7],col8[(i>>2)&0x7],col4[i&3],i<255?',':' ');

	for (b=0;b<4;b++) 
		for (g=0;g<8;g++) 
			for (r=0;r<8;r++)  {
				printf ("{%d,%d,%d}%c\n",col8[r],col8[g],col4[b],i<255?',':' ');
/*				colors[i].r=r*(255.0f/3.0f)/255.0f;
				colors[i].g=g*(255.0f/7.0f)/255.0f;
				colors[i].b=b*(255.0f/7.0f)/255.0f;*/



				i++;
			}

//	qsort(&colors,256,sizeof(rgb),cmp);

	for (i=0;i<256;i++)
//		printf ("{%.0f,%.0f,%.0f}%c //%f %f %f\n",colors[i].r*255,colors[i].g*255,colors[i].b*255,i<255?',':' ',colors[i].r,colors[i].g,colors[i].b);
//		printf ("%.0f\t%.0f\t%.0f\n",colors[i].r*255,colors[i].g*255,colors[i].b*255,i<255?',':' ',colors[i].r,colors[i].g,colors[i].b);

	return 0;
}
		
