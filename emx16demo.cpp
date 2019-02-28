#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "types.h"
#include "memory.h"
#include "png.h"
#include "vtext.h"
#include "vga.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <iostream>

using namespace std;

typedef struct {
	image box;
	image boxc;
	image mytext;
	vga display;
	unsigned int x,y,w,h,dx,dy;
	int rot,rotx;
	bool endprogram;
} context;


void animate(void *arg)
{
		context *ctx=(context *)arg;

		ctx->display.syncsprites();
		ctx->box.rotate(ctx->boxc,ctx->rot);
		ctx->display.drawsprite(ctx->x,ctx->y,ctx->boxc);
		ctx->display.drawsprite(ctx->display.width()-ctx->mytext.width(),2,ctx->mytext);

		ctx->display.update();
		ctx->rot+=ctx->rotx;
		if(ctx->rot>360) ctx->rot=0;
		if(ctx->rot<0) ctx->rot=360;

		ctx->x+=ctx->dx;
		ctx->y+=ctx->dy;

		if(ctx->x>ctx->display.width()-ctx->w-1 || ctx->x<1) {
			ctx->dx*=-1;
			ctx->rotx*=-1;
			ctx->rotx=((ctx->rotx>0)-(ctx->rotx<0))*(rand()%5+1);
		}

		if(ctx->y>ctx->display.height()-ctx->h-1 || ctx->y<1) {
			ctx->dy*=-1;
			ctx->rotx*=-1;
			ctx->rotx=((ctx->rotx>0)-(ctx->rotx<0))*(rand()%5+1);
		}

		if (ctx->display.kbhit()) {
			printf("Exit...\n");
#ifdef __EMSCRIPTEN__
			emscripten_cancel_main_loop();
#else
			ctx->endprogram=true;
#endif
		}
}

int main(void)
{
	context ctx;

	ctx.endprogram=false;
	ctx.rot=0,ctx.rotx=(rand()%5+1);

	ctx.dx=1;
	ctx.dy=1;
	ctx.x=0;
	ctx.y=0;

	printf("Starting...\n");

	ctx.display.graphmode(vga::X16);
	ctx.display.initsprites();
	ctx.display.cls();
	ctx.display.setpalette(palette::CGA_PAL);
	canvas::setdefpalette(ctx.display.getpalette());

	png mariopng;
	image mario;
	image bg;

	printf("Loading...\n");
	if (!mariopng.load("emscripten/assets/dopefish.png")) {
		printf("Error loading sprite\n");
		return(1);
	}

	printf("Convert...\n");
	mariopng.convert(mario);
	printf("Free...\n");
	mariopng.free();

	/*
	printf("Math...\n");
	double sides=(double)mario.width()*(double)mario.width()+(double)mario.height()*(double)mario.height();
	int hyp=(int) sqrt(sides);
	ctx.w=hyp;
	ctx.h=hyp;
	*/
	ctx.w=mario.width();
	ctx.h=mario.height();

	ctx.box.size(ctx.w,ctx.h);

	printf("Building...\n");

	vtext text(ctx.display.width(),ctx.display.height(),0);
	ctx.mytext.setbg(14);
	text.drawtext(ctx.mytext,"KEEN 4",13);

	if (!mariopng.load("emscripten/assets/keen4.png")) {
		printf("Errro loading bg\n");
		return(1);
	}

	mariopng.convert(bg);
	mariopng.free();
	ctx.display.drawimage(0,0,bg);
	ctx.display.syncsprites();
	bg.free();


	ctx.box.setbg(mario.getbg());
	ctx.box.clear();
	ctx.box.copypalette(mario);
	ctx.box.drawimage((ctx.box.width()-mario.width())/2,(ctx.box.height()-mario.height())/2,mario);

	ctx.boxc=ctx.box;

	ctx.display.drawsprite(ctx.x,ctx.y,ctx.boxc);
	printf("Update\n");
	ctx.display.update();

#ifdef __EMSCRIPTEN__
    int simulate_infinite_loop = 1;

    emscripten_set_main_loop_arg(animate, &ctx, -1, simulate_infinite_loop);
#else
    while(!ctx.endprogram)
		animate(&ctx);
#endif

    /**
     * If simulate_infinite_loop = 0, emscripten_set_main_loop_arg won't block
     * and this code will run straight away.
     * If simulate_infinite_loop = 1 then this code will not be reached
     */
    printf("quitting...\n");

	ctx.display.textmode();

	return 0;
}

