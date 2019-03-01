# VGALIB

This is a cross platform graphics library written to compile for DOS, Linux+SDL, and Web (emscripten+SDL).

This project started out as a simple screensaver written in 1995, then brought back to life in late 2018 after discovering an old hard drive with all of my old DOS files.

The library and programs compile under Borland C++ 4.0 and later.  If std::string support is removed, it will compile under BC3.1, since BC3.1 is missing cstring.h.

The library supports VGA, VGA 160x100 16 color text mode, CGA 160x100 16 color text mode, TEXT as graphics (80x25), and MDA as graphics (80x25).

Video update is coordinated with vsync and the library detects if you have a VGA or CGA card and is able to perform the correct setup for 160x100 text mode.

The DOS portions of this code run in Dosbox, however BC4.0 doesn't run entirely in Dosbox due to DPMI issues and emulation problems.

To compile this for DOS, I've been running BC4.0/4.5/5.0 under a Windows 2000 VM, but you could create a DOS VM just the same or use QEMU.

The code uses inline assembler to speed up certain operations when compiled for DOS.  This inline assembler is written for i386 or later, in the future 16bit assembler will be selectable via compile time define.

Under Linux it uses SDL2 to render the graphics to the screen.  The library is versatile because it implements an 8bpp off screen buffer for all graphics operations, then it translates or copies these offscreen buffers to the screen buffer during vsync.

There are 2 offscreen buffers, the primary buffer which is drawn to with vga::drawimage, and the sprite buffer which is drawn to with vga::drawsprite.

When compositing the output, vga::syncsprites copies the \_os\_buffer to the \_sprite\_buffer, then sprites are drawn onto the \_sprite\_buffer with the background color used as a mask.  Finally, the \_sprite\_buffer is translated or copied to the \_screen buffer.

The rotate and scale routines are quite intensive, so it's best to not perform a lot of realtime scaling or rotation if targetting DOS.

On modern systems and web browsers, performance seems to be very good and CPU usage is much less than on emulators.

--Perry Harrington

## Few notes about the code enclosed

png.h/cpp are a full standalone PNG decoder.  libpng doesn't compile for 16bit DOS, so I wrote my own PNG decoder.  The decoder first loads a PNG into the PNG class, then you call png::convert to convert it into an image object.  This will decode indexed images, grayscale, and RGB truecolor.  Transparency is respected, but note that if the source image shares a color in the image with the color marked transparent in the palette, it won't render like you expect.  PNG uses the deflate algorithm, the correct header and LIB files are included to compile with the HUGE memory model on DOS.  If you target something else, you'll need to download and build ZLIB for that model.  I found my color quantizing algorithm produces a nicer result than GIMP, however if you target DOS, I recommend converting your images to indexed color mode prior to building the application, since RGB conversion is very intensive.  For images less than 262144 (512x512) pixels, a palette compare/lookup is done for every pixel, for larger images a cache is built for all possible colors, which consumes 256KB of memory.  Since the library doesn't support larger display modes, the less-memory hungry method is more cycle hungry -- but this can all be ignored if you pre-convert the images.  I have provided the VGA palette as VGA.gpl and modified-SNES pallete as RRRGGGBBw.gpl, as well as SNES.gpl, which implements a straight RRRGGGBB 8Bpp direct mapped palette.

direct8bpp.c is a palette generator.  I spent a lot of time toying with different palettes.  If you have bold colors without many grays, the modified SNES palette does a good job.  To modify the palette I pushed the steps around to get 4 shades of gray, which the SNES pallette has none.  You can see the tweaks I tried out in the palette\_values.ods spreadsheet.
