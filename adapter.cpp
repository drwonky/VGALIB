
#include "types.h"
#include "string.h"
#include "adapter.h"
#include "vga.h"
#include "sdl.h"

#ifdef __GNUC__
#include <iostream>

using namespace std;
#endif

adapter::adapter(void)
{
	colors=0;
	_width=0;
	_height=0;
	_palette=NULL;
	_palette_size=0;
	_cur_palette=palette::NONE;
	_row_bytes=0;
	bpp=0;
	Bpp=0;
	SR=0;
	planes=0;
	_buffer=NULL;
	buf_size=0;
	_display=NULL;
	vmode=UNDEF;
}

adapter::~adapter()
{
	if (_display) delete [] _display;
}

bool adapter::setpalette(palette::pal_type pal)
{
	_palette_size=palette::palettes[pal].palette_entries;
	_cur_palette=pal;
	_palette=new palette::pal_t[_palette_size];

	if (_palette == NULL) return false;

	memcpy(_palette,palette::palettes[pal].pal,sizeof(palette::pal_t)*_palette_size);

	return true;
}

ptr_t adapter::allocate_screen_buffer()
{
	ptr_t buf = new unsigned far char [buf_size];
	return buf;
}

bool adapter::kbhit(void) // @suppress("No return")
{
#ifdef __BORLANDC__
	return kbhit();
#else
	return false;
#endif
}

int adapter::getch(void) // @suppress("No return")
{
#ifdef __BORLANDC__
	return getchar();
#else
	return 0;
#endif
}

adapter::Adapters adapter::detect(void)
{
#ifdef __BORLANDC__
	unsigned char status;
	unsigned char active;
	unsigned char switches;

	_AH=0xF;
	_AL=0;
	geninterrupt(0x10);		/* get current display settings from BIOS */

	if (_AL == 0x07) {		/* Check that this is BW 80x25 */

		/* Check if there is 32KB of memory from B000-B7FF, if there is, it's Hercules Monochrome */
		unsigned char far *check_ptr = MK_FP(0xB7FF,0);
		*check_ptr = 0xA5;
		if (*check_ptr == 0xA5) {
			*check_ptr = 0;
			return HERC;
		}

		return MDA;
	}

	_AH=0x12;
	_AL=0;
	_BL=0x10;
	geninterrupt(0x10);		/* get EGA switch settings */

	switches = _CL;

	_AH=0x1a;
	_AL=0;
	geninterrupt(0x10);		/* get current display settings from BIOS (VGA only) */

	active = _BL;
	status = _AL;

	if (status == 0x1a && (active == 0x07 || active == 0x08)) {  /* VGA color or mono*/

		return VGA;	/* VGA */

	} else if ( /* EGA */
				switches == 0x6 ||	/* CGA w/CGA 40x25 */
				switches == 0x7 ||	/* CGA w/CGA 80x25 */
				switches == 0x8 ||	/* EGA w/CGA 80x25 */
				switches == 0x9 ||	/* EGA w/ECD 80x25 */
				switches == 0xB 	/* EGA w/MONO */
				) {

		return EGA;	/* EGA */

	} else { /* CGA does not have an attribute controller register, only mode controller */

		_AH=0xF;
		_AL=0;
		geninterrupt(0x10);		/* get current display settings from BIOS */

		if (_AL != 0x07) {		/* Check that this is not BW 80x25 */

			return CGA;	/* CGA */

		}
	}

#else
#ifdef USESDL
	return SDL;
#endif
#endif

	return NONE;
}

adapter *adapter::init(Adapters card)
{
	switch(card) {
	case VGA:
		return new vga();
		break;
	case SDL:
		return new sdl();
		break;
	default:
		return NULL;
		break;
	}
}
