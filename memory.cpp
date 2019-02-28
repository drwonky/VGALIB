#include <stdlib.h>
#include "memory.h"

#ifdef __BORLANDC__
#include <dos.h>
#else
#include "string.h"
#endif

void memory::mask_memcpy(ptr_t dest, ptr_t src, size_t size, unsigned char mask)
{

#ifdef __BORLANDC__
	_DI=FP_OFF(dest);
	_SI=FP_OFF(src);
	_BX=FP_SEG(src);
	_DS=_BX;
	_AX=FP_SEG(dest);
	_ES=_AX;
	_CX=size;
	_BL=mask;

bytecopy:
	asm {
		lodsb // 5
		cmp	al,bl // 2
		jz	nobytecopy // 3
		stosb // 4
		dec	cx
		jnz bytecopy // 15
		jmp	endbytecopy
		}
nobytecopy:
	asm {
		inc	di
		dec	cx
		jnz bytecopy
		}
endbytecopy:
#else
	ptr_t s,p,e;

//	ptr_t *sp[2];
//	sp[0] = &s;
//	sp[1] = &p;
//	p = dest;
//	s = src;
//	e = dest + size;
//	do
//	{
//		*p = *(*sp[*s == mask]);
//		p++;
//		s++;
//	} while (p < e);

//	p=dest;
//	s=src;
//	e=dest+size;
//	while(p<e) {
//		if(*s!=mask) *p=*s;
//		p++;
//		s++;
//	}

	p=dest;
	s=src;
	e=dest+size;
	do {
		if(*s!=mask) *p=*s;
		p++;
		s++;
	} while(p<e);
#endif
}

void memory::fast_memcpy(void *dest, void *src, size_t size)
{

#ifdef __BORLANDC__
	if (size > 4) { // long way default, byte copy the long way
		_DI=FP_OFF(dest);
		_SI=FP_OFF(src);
		_BX=FP_SEG(src);
		_DS=_BX;
		_AX=FP_SEG(dest);
		_ES=_AX;

		asm {
			mov bx,size	// count down size
			mov cx, di  // dest
			test cx, 1  // byte offset?
			jz pre_word // no, goto work offset
			movsb       // copy byte
			dec bx      // 1 byte copied
			}

pre_word:
		asm {
			test cx, 2  // word offset?
			jz pre_dword// no, go to dword
			movsw       // copy word
			sub bx,2    // 2 bytes copied
			}
pre_dword:
		asm {
			movsd       // addr must be dw aligned
			sub bx,4    // 4 bytes copied, we are dw aligned now
			mov cx,bx
			shr cx,2    // divide by 4 to get dw remaining
			jz post     // no dwords left
			mov ax,cx
			shl ax,2
			sub bx,ax   // subtract dwords copied from total
			cld
			rep movsd   // copy dwords
			}
post:
		asm {
			test bx,2   // test number of words remaining
			jz post_byte// no words remaining
			movsw       // copy word
			}
post_byte:
		asm {
			test bx,1   // test number of bytes remaining
			jz post_end // no bytes remaining
			movsb       // copy bytes
		}
post_end:


	} else {
		_CX=size;
		_DI=FP_OFF(dest);
		_SI=FP_OFF(src);
		_BX=FP_SEG(src);
		_DS=_BX;
		_AX=FP_SEG(dest);
		_ES=_AX;
		asm {
			rep movsb
		}

	}
#else
	memcpy(dest,src,size);
#endif

}

void memory::blit(ptr_t dest, ptr_t src, size_t size)
{
#ifdef __BORLANDC__
		_DI=FP_OFF(dest);
		_SI=FP_OFF(src);
		_BX=FP_SEG(src);
		_DS=_BX;
		_AX=FP_SEG(dest);
		_ES=_AX;

		asm {
			mov cx,size
			shr cx,2
			rep movsd
		}
#else
	memcpy(dest,src,size);
#endif
}
