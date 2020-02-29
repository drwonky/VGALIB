BCC = bcc
TLINK = tlink
BCFLAGS = -a- -3 -Fs- -mh -O2 -v
# -Ie:\bc45\include -Le:\bc45\lib

CFLAGS = -Wall -Wextra -Wpedantic -Wno-unused-parameter -ggdb -std=c++98
# -DTEST -DDEBUG

CC = gcc
CXX = g++

EXX = em++
EMFLAGS = -O2 --preload-file emscripten/assets --shell-file shell_minimal.html -s USE_ZLIB=1 -s USE_SDL=2

SRCDEP = emdemo.cpp emx16demo.cpp adapter.cpp sdl.cpp vga.cpp png.cpp image.cpp memory.cpp vtext.cpp palettes.cpp canvas.cpp
SRC = vga.cpp sdl.cpp adapter.cpp png.cpp image.cpp memory.cpp vtext.cpp palettes.cpp canvas.cpp
OBJS = emdemo.o emx16demo.o vgademo.o vga.o adpater.o sdl.o png.o image.o memory.o vtext.o palettes.o canvas.o
BIN = emdemo emx16demo png vgademo VGA.EXE
SDLFLAGS = $(shell sdl2-config --cflags --libs)

all: emdemo emx16demo emscripten/emdemo.html emscripten/emx16demo.html

.cpp.o:
	$(CXX) $(CFLAGS) -c -o $@ $<
	
.c.o:
	$(CC) $(CFLAGS) -o $@ $<

.cpp.obj:
	$(BCC) $(BCFLAGS) -c $<
	
depend: $(SRCDEP)
	makedepend -- $(CFLAGS) -- $^
	
vtext.obj: vtext.cpp fonts.h

memory.obj: memory.cpp
	$(BCC) $(BCFLAGS) -B -c memory.cpp

png.obj: png.cpp image.obj canvas.obj memory.obj zlib.h zconf.h
#	$(BCC) $(BCFLAGS) -DDEBUG -c $**

cga.obj: cga.cpp
	$(BCC) $(BCFLAGS) -B -c $**
	
ega.obj: ega.cpp
	$(BCC) $(BCFLAGS) -B -c $**
	
vga.obj: vga.cpp
	$(BCC) $(BCFLAGS) -B -c $**
	
pngtest.o: png.cpp
	$(CXX) $(CFLAGS) -DTEST -DDEBUG -c -o $@ $^

matrix: matrix.cpp
	$(CXX) $(CFLAGS) -DTEST -o $@ $^

png: image.o memory.o pngtest.o palettes.o canvas.o
	$(CXX) $(CFLAGS) -DTEST -DDEBUG -o $@ $^ -lz

vgademo: vgademo.o vga.o sdl.o adapter.o image.o memory.o png.o vtext.o palettes.o canvas.o
	$(CXX) $(CFLAGS) $(SDLFLAGS) -o $@ $^ -lz

emdemo: emdemo.o vga.o image.o memory.o png.o vtext.o palettes.o canvas.o
	$(CXX) $(CFLAGS) $(SDLFLAGS) -o $@ $^ -lz

emx16demo: emx16demo.o vga.o image.o memory.o png.o vtext.o palettes.o canvas.o
	$(CXX) $(CFLAGS) $(SDLFLAGS) -o $@ $^ -lz

emscripten/emdemo.html: emdemo.cpp $(SRC)
	$(EXX) $(EMFLAGS) -o $@ $^ 
	
emscripten/emx16demo.html: emx16demo.cpp $(SRC)
	$(EXX) $(EMFLAGS) -o $@ $^ 
	
png.exe: png.cpp memory.obj image.cpp zlib.h zconf.h zlib_h.lib
	bcc -DDEBUG -DTEST -3 -Fs- -mh -v png.cpp memory.obj image.cpp zlib_h.lib

#vgademo.exe: adapter.obj memory.obj canvas.obj image.obj png.obj zlib_h.lib vtext.obj palettes.obj vgademo.obj vga.obj 
#	$(BCC) $(BCFLAGS) vgademo.obj adapter.obj vga.obj memory.obj canvas.obj image.obj png.obj zlib_h.lib vtext.obj palettes.obj
LIBS=zlib_h.lib
DEMOOBJS = adapter.obj memory.obj canvas.obj image.obj png.obj vtext.obj palettes.obj vga.obj ega.obj cga.obj 
DEMOS = vgademo.exe cgademo.exe egademo.exe
VGAOBJS = vgademo.obj $(DEMOOBJS)
vgademo.exe: $(VGAOBJS)
#	$(BCC) -e vgademo.exe $(VGAOBJS) zlib_h.lib
	$(BCC) $(BCFLAGS) -e$< $** $(LIBS)
	
CGAOBJS = cgademo.obj $(DEMOOBJS)
cgademo.exe: $(CGAOBJS)
	$(BCC) $(BCFLAGS) -e$< $** $(LIBS)
	
EGAOBJS = egademo.obj $(DEMOOBJS)
egademo.exe: $(EGAOBJS)
	$(BCC) $(BCFLAGS) -e$< $** $(LIBS)
	
demos: $(DEMOS)

dclean:
	del egademo.obj vgademo.obj cgademo.obj $(DEMOOBJS) $(DEMOS)

clean:
	rm -f $(OBJS) $(BIN)

# DO NOT DELETE

