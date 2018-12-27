class bitmaps {
unsigned char *bitmap;
unsigned long width;
unsigned long r_width;
unsigned long height;
unsigned long size;
unsigned long pixels;

public:
void getbitmap(char *filename);
void blitmap(vga_mono &disp, int x, int y);
~bitmaps(void);
void flip(void);
};

void bitmaps::blitmap(vga_mono &disp, int x, int y)
{
	unsigned int x1,y1,tx=(x+width),ty=(y+height);
	unsigned int maxx=disp.maxx(),maxy=disp.maxy();
	for(y1=y;y1<ty&&y1<maxy;y1++)
	{
		for(x1=x;x1<tx&&x1<maxx;x1++)
		{
			disp.setpixel(x1,y1,bitmap[((y1-y)*(r_width))+(x1-x)]);
		}
	}
}

void bitmaps::getbitmap(char *filename)
{
	unsigned long temp,temp1;
	unsigned int bpp;
	unsigned char *tc;
	FILE *in;
	if((in=fopen(filename,"rb"))==NULL)
	{
		exitinstr("Error opening BMP file\n");
	}
	fseek(in,28L,0);
	fread(&bpp,2,1,in);
	if(bpp!=1)
	{
		exitinstr("Error: BMP must be 1 bit per pixel\n");
	}
	fseek(in,18L,0);
	fread(&width,4,1,in);
	fread(&height,4,1,in);
	r_width=(((width+31)/32)*32);
	pixels=r_width*height;
	size=(pixels+7)/8;
	size=(size+3)/4;
	size*=4;
	temp=pixels;temp1=size;
	if((bitmap=(unsigned char *) malloc(size*8))==NULL)
	{
		printf("Error allocating %lu bytes for BMP\n",size*8);
		exitinstr("");
	}
	tc=bitmap;
	fseek(in,62L,0);

	while (temp > 0)
	{
		temp1--;
		int x=fgetc(in);
		int m=0x80;
		while((temp>0) && (m!=0))
		{
			tc[0]=(x & m) ? 1 : 0;
			tc++;
			m >>=1;
			temp--;
		}
	}
	while (temp1-- >0) fgetc(in);
	printf("%lu (%lu) x %lu = %lu (%lu)\n",width,r_width,height,size,(r_width*height)/8);
	delay(200);
	fclose(in);
}

void bitmaps::flip(void)
{
	unsigned long y;
	unsigned char *swap;

	swap=new unsigned char [r_width];
	for(y=0;y<(height/2);y++)
	{
			memcpy(swap,&bitmap[y*r_width],r_width);
			memcpy(&bitmap[y*r_width],&bitmap[(height-y)*r_width],r_width);
			memcpy(&bitmap[(height-y)*r_width],swap,r_width);
	}
	delete swap;
}

bitmaps::~bitmaps(void)
{
	free(bitmap);
}
