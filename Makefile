BCC = bcc
BCFLAGS = -a- -3 -Fs- -mh -v

CFLAGS = -ggdb -std=c++98 -O2
# -DTEST -DDEBUG

CC = gcc
CXX = g++

SRC = png.cpp image.cpp memory.cpp vtext.cpp palettes.cpp canvas.cpp
OBJS = png.o pngtest.o image.o memory.o vtext.o vga.o palettes.o canvas.o
BIN = png vga VGA.EXE
SDLFLAGS = $(shell sdl2-config --cflags --libs)

all:

.cpp.o:
	$(CXX) $(CFLAGS) -c -o $@ $<
	
.c.o:
	$(CC) $(CFLAGS) -o $@ $<

.obj.cpp:
	$(BCC) $(BCFLAGS) -c $<
	
depend: $(SRC)
	makedepend -- $(CFLAGS) -- $^
	
vtext.obj: vtext.cpp fonts.h

memory.obj: memory.cpp
	$(BCC) $(BCFLAGS) -B -c memory.cpp

png.obj: png.cpp image.obj memory.obj zlib.h zconf.h

image.obj: image.cpp

vga.obj: vga.cpp
	$(BCC) $(BCFLAGS) -B -c vga.cpp
	
pngtest.o: png.cpp
	$(CXX) $(CFLAGS) -DTEST -DDEBUG -c -o $@ $^

matrix: matrix.cpp
	$(CXX) $(CFLAGS) -DTEST -o $@ $^

png: image.o memory.o pngtest.o palettes.o
	$(CXX) $(CFLAGS) -DTEST -DDEBUG -o $@ $^ -lz

vga: vga.o image.o memory.o png.o vtext.o palettes.o canvas.o
	$(CXX) $(CFLAGS) $(SDLFLAGS) -o $@ $^ -lz
	
png.exe: png.cpp memory.obj image.cpp zlib.h zconf.h zlib_h.lib
	bcc -DDEBUG -DTEST -3 -Fs- -mh -v png.cpp memory.obj image.cpp zlib_h.lib

vga.exe: memory.obj image.obj png.obj vga.obj zlib_h.lib vtext.obj
	$(BCC) $(BCFLAGS) vga.obj image.obj memory.obj png.obj vtext.obj zlib_h.lib
	
clean:
	rm -f $(OBJS) $(BIN)

# DO NOT DELETE

png.o: /usr/include/stdio.h /usr/include/bits/libc-header-start.h
png.o: /usr/include/features.h /usr/include/stdc-predef.h
png.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
png.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
png.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
png.o: /usr/include/bits/types/__FILE.h /usr/include/bits/types/FILE.h
png.o: /usr/include/bits/libio.h /usr/include/bits/_G_config.h
png.o: /usr/include/bits/types/__mbstate_t.h /usr/include/bits/stdio_lim.h
png.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
png.o: /usr/include/bits/types/locale_t.h
png.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
png.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
png.o: /usr/include/bits/waitstatus.h /usr/include/bits/floatn.h
png.o: /usr/include/bits/floatn-common.h /usr/include/sys/types.h
png.o: /usr/include/bits/types/clock_t.h /usr/include/bits/types/clockid_t.h
png.o: /usr/include/bits/types/time_t.h /usr/include/bits/types/timer_t.h
png.o: /usr/include/bits/stdint-intn.h /usr/include/endian.h
png.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
png.o: /usr/include/bits/byteswap-16.h /usr/include/bits/uintn-identity.h
png.o: /usr/include/sys/select.h /usr/include/bits/select.h
png.o: /usr/include/bits/types/sigset_t.h
png.o: /usr/include/bits/types/__sigset_t.h
png.o: /usr/include/bits/types/struct_timeval.h
png.o: /usr/include/bits/types/struct_timespec.h /usr/include/sys/sysmacros.h
png.o: /usr/include/bits/sysmacros.h /usr/include/bits/pthreadtypes.h
png.o: /usr/include/bits/thread-shared-types.h
png.o: /usr/include/bits/pthreadtypes-arch.h /usr/include/alloca.h
png.o: /usr/include/bits/stdlib-float.h image.h types.h /usr/include/stdint.h
png.o: /usr/include/bits/wchar.h /usr/include/bits/stdint-uintn.h canvas.h
png.o: palettes.h /usr/include/zlib.h zconf.h /usr/include/limits.h
png.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
png.o: /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h png.h
png.o: /usr/include/errno.h /usr/include/bits/errno.h
png.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
png.o: /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h
image.o: /usr/include/math.h /usr/include/bits/libc-header-start.h
image.o: /usr/include/features.h /usr/include/stdc-predef.h
image.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
image.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
image.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
image.o: /usr/include/bits/math-vector.h
image.o: /usr/include/bits/libm-simd-decl-stubs.h /usr/include/bits/floatn.h
image.o: /usr/include/bits/floatn-common.h
image.o: /usr/include/bits/flt-eval-method.h /usr/include/bits/fp-logb.h
image.o: /usr/include/bits/fp-fast.h
image.o: /usr/include/bits/mathcalls-helper-functions.h
image.o: /usr/include/bits/mathcalls.h /usr/include/bits/iscanonical.h
image.o: /usr/include/stdio.h /usr/include/bits/types/__FILE.h
image.o: /usr/include/bits/types/FILE.h /usr/include/bits/libio.h
image.o: /usr/include/bits/_G_config.h /usr/include/bits/types/__mbstate_t.h
image.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
image.o: /usr/include/string.h /usr/include/bits/types/locale_t.h
image.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h types.h
image.o: /usr/include/stdint.h /usr/include/bits/wchar.h
image.o: /usr/include/bits/stdint-intn.h /usr/include/bits/stdint-uintn.h
image.o: memory.h image.h canvas.h palettes.h sincos.h
memory.o: /usr/include/stdlib.h /usr/include/bits/libc-header-start.h
memory.o: /usr/include/features.h /usr/include/stdc-predef.h
memory.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
memory.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
memory.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
memory.o: /usr/include/bits/floatn.h /usr/include/bits/floatn-common.h
memory.o: /usr/include/sys/types.h /usr/include/bits/types.h
memory.o: /usr/include/bits/typesizes.h /usr/include/bits/types/clock_t.h
memory.o: /usr/include/bits/types/clockid_t.h
memory.o: /usr/include/bits/types/time_t.h /usr/include/bits/types/timer_t.h
memory.o: /usr/include/bits/stdint-intn.h /usr/include/endian.h
memory.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
memory.o: /usr/include/bits/byteswap-16.h /usr/include/bits/uintn-identity.h
memory.o: /usr/include/sys/select.h /usr/include/bits/select.h
memory.o: /usr/include/bits/types/sigset_t.h
memory.o: /usr/include/bits/types/__sigset_t.h
memory.o: /usr/include/bits/types/struct_timeval.h
memory.o: /usr/include/bits/types/struct_timespec.h
memory.o: /usr/include/sys/sysmacros.h /usr/include/bits/sysmacros.h
memory.o: /usr/include/bits/pthreadtypes.h
memory.o: /usr/include/bits/thread-shared-types.h
memory.o: /usr/include/bits/pthreadtypes-arch.h /usr/include/alloca.h
memory.o: /usr/include/bits/stdlib-float.h memory.h types.h
memory.o: /usr/include/stdint.h /usr/include/bits/wchar.h
memory.o: /usr/include/bits/stdint-uintn.h /usr/include/string.h
memory.o: /usr/include/bits/types/locale_t.h
memory.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
vtext.o: /usr/include/string.h /usr/include/bits/libc-header-start.h
vtext.o: /usr/include/features.h /usr/include/stdc-predef.h
vtext.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
vtext.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
vtext.o: /usr/include/bits/types/locale_t.h
vtext.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
vtext.o: /usr/include/stdio.h /usr/include/bits/types.h
vtext.o: /usr/include/bits/typesizes.h /usr/include/bits/types/__FILE.h
vtext.o: /usr/include/bits/types/FILE.h /usr/include/bits/libio.h
vtext.o: /usr/include/bits/_G_config.h /usr/include/bits/types/__mbstate_t.h
vtext.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
vtext.o: image.h types.h /usr/include/stdint.h /usr/include/bits/wchar.h
vtext.o: /usr/include/bits/stdint-intn.h /usr/include/bits/stdint-uintn.h
vtext.o: canvas.h palettes.h vtext.h fonts.h
palettes.o: palettes.h
canvas.o: /usr/include/stdlib.h /usr/include/bits/libc-header-start.h
canvas.o: /usr/include/features.h /usr/include/stdc-predef.h
canvas.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
canvas.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
canvas.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
canvas.o: /usr/include/bits/floatn.h /usr/include/bits/floatn-common.h
canvas.o: /usr/include/sys/types.h /usr/include/bits/types.h
canvas.o: /usr/include/bits/typesizes.h /usr/include/bits/types/clock_t.h
canvas.o: /usr/include/bits/types/clockid_t.h
canvas.o: /usr/include/bits/types/time_t.h /usr/include/bits/types/timer_t.h
canvas.o: /usr/include/bits/stdint-intn.h /usr/include/endian.h
canvas.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
canvas.o: /usr/include/bits/byteswap-16.h /usr/include/bits/uintn-identity.h
canvas.o: /usr/include/sys/select.h /usr/include/bits/select.h
canvas.o: /usr/include/bits/types/sigset_t.h
canvas.o: /usr/include/bits/types/__sigset_t.h
canvas.o: /usr/include/bits/types/struct_timeval.h
canvas.o: /usr/include/bits/types/struct_timespec.h
canvas.o: /usr/include/sys/sysmacros.h /usr/include/bits/sysmacros.h
canvas.o: /usr/include/bits/pthreadtypes.h
canvas.o: /usr/include/bits/thread-shared-types.h
canvas.o: /usr/include/bits/pthreadtypes-arch.h /usr/include/alloca.h
canvas.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
canvas.o: /usr/include/bits/types/locale_t.h
canvas.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
canvas.o: canvas.h types.h /usr/include/stdint.h /usr/include/bits/wchar.h
canvas.o: /usr/include/bits/stdint-uintn.h palettes.h sincos.h memory.h
