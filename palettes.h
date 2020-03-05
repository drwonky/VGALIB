#ifndef VGA_PAL_H
#define VGA_PAL_H

class palette
{
public:
	typedef struct
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	} pal_t;

	typedef struct
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	} pala_t;

	enum pal_type
	{
		TEXT_PAL,
		CGA0_PAL,
		CGA1_PAL,
		CGA2_PAL,
		CGA0HI_PAL,
		CGA1HI_PAL,
		CGA2HI_PAL,
		BW_PAL,
		VGA_PAL,
		RGB_PAL,
		WEB_PAL,
		X11_PAL,
		CUSTOM,
		NONE = 255
	};

	static const pal_t vga_pal[];
	static const pal_t bw_pal[];
	static const pal_t text_pal[];
	static const pal_t cga0_pal[];
	static const pal_t cga1_pal[];
	static const pal_t cga2_pal[];
	static const pal_t cga0hi_pal[];
	static const pal_t cga1hi_pal[];
	static const pal_t cga2hi_pal[];
	static const pal_t rgb8bpp_pal[];
	static const pal_t web_pal[];
	static const pal_t x11_pal[];

	typedef struct
	{
		pal_type set;
		int palette_entries;
		const pal_t *pal;
	} palette_def;

	static const palette_def palettes[];

};

#endif
