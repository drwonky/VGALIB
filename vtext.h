#include "fonts.h"

class vtext {
protected:
int font_num;
fontdef *font;
int max_width;
int max_height;
unsigned char bgcolor;

public:
vtext();
~vtext();
vtext(int width, int height, unsigned char background);

bool drawtext(image& img, const char *string, unsigned char color);
};

