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

