/*
 * vga.h
 *
 *  Created on: Jan 3, 2019
 *      Author: pedward
 */

#ifndef SDL_H_
#define SDL_H_

#ifdef __GNUC__
#include <SDL2/SDL.h>

#include "image.h"
#include "adapter.h"

class sdl: public adapter
{
protected:
	SDL_Window* _window;
	SDL_Surface* _screen;
	SDL_Renderer* _renderer;
	SDL_Surface* _render;
	SDL_Texture* _texture;
	SDL_Event _event;
	unsigned int _sdlscale;

	static const adapter::video_mode video_modes[];
	static bool SDLonce;

protected:
	bool sdlmode(Mode mode);

public:
	sdl(void);
	~sdl(void);
	bool setup(void);
	bool setpalette(palette::pal_type pal);
	bool setpalette(palette::pal_t *pal, int palette_entries);
	bool graphmode(Mode mode);
	Mode getmode(void);
	bool textmode(void) { return false; }
	void cls(void);
	void update(void);
	void vsync(void) {}
	void translate(unsigned char far *src);
	bool kbhit(void);
	int getch(void);
	int getEvents(int ms);

};
#endif

#endif /* SDL_H_ */
