#ifndef VGA_PAL_H
#define VGA_PAL_H

class palette {
public:
	typedef struct {
		unsigned char r;
		unsigned char g;
		unsigned char b;
	} pal_t;

	enum pal_type { CGA_PAL, VGA_PAL };

	static const pal_t vga_pal[];
	static const pal_t cga_pal[];

	typedef struct {
		pal_type set;
		int palette_entries;
		const pal_t *pal;
	} palette_def;

	static const palette_def palettes[];

};

#endif
