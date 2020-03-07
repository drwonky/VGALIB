#ifndef FONT_H
#define FONT_H

// Font: 8X8!FONT.pf

typedef struct {
int font_height;
int font_width;
char bytes_per_char;
unsigned char font[2048];
} fontdef;

extern const fontdef fonts[];

#endif
