enum bool { false, true };

class v_display {
protected:
unsigned char far *buffer;
unsigned char far *saved_buffer;
unsigned int *tsvscrn;
unsigned int x_w;
unsigned int y_h;
unsigned int row_bytes;
unsigned char temp;
unsigned char bit;

/*
public:
virtual bool graphmode(void);
virtual bool textmode(void);
virtual void setpixel(int x, int y, unsigned char visible);
virtual unsigned char getpixel(int x, int y);
virtual void cls(void);
virtual unsigned int width(void);
virtual unsigned int height(void);
virtual void save_buffer(void);
virtual void restore_buffer(void);
virtual int copy_buffer(unsigned char *dest, unsigned char *src);
*/
};
