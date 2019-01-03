all: vga.exe

memory.obj: memory.cpp
	bcc -c -B -3 -Fs- -mh -v memory.cpp

png.obj: png.cpp image.obj memory.obj zlib.h zconf.h
	bcc -c -3 -Fs- -mh -v png.cpp

image.obj: image.cpp
	bcc -c -3 -Fs- -mh -v image.cpp

vga.obj: vga.cpp
	bcc -c -B -3 -Fs- -mh -v vga.cpp

png.exe: png.cpp memory.obj image.cpp zlib.h zconf.h zlib_h.lib
	bcc -DDEBUG -DTEST -3 -Fs- -mh -v png.cpp memory.obj image.cpp zlib_h.lib

vga.exe: memory.obj image.obj png.obj vga.obj zlib_h.lib
	bcc -3 -Fs- -mh -v vga.obj image.obj memory.obj png.obj zlib_h.lib
