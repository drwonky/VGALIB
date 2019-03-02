#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
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
	image bg;
	image box;
	image boxc;
	image mytext;
	vga display;
	unsigned int x,y,w,h,dx,dy;
	int rot,rotx;
} context;


void animate(void *arg)
{
		context *ctx=(context *)arg;

		ctx->display.screen.drawimage(0,0,ctx->bg);
		ctx->display.screen.drawsprite(ctx->display.width()-ctx->mytext.width(),2,ctx->mytext);
		ctx->box.rotate(ctx->boxc,ctx->rot);
		ctx->display.screen.drawsprite(ctx->x,ctx->y,ctx->boxc);

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

#ifdef __EMSCRIPTEN__
		if (ctx->display.kbhit()) {
			printf("Exit...\n");
			emscripten_cancel_main_loop();
		}
#endif
}

int main(void)
{
	context ctx;

	ctx.rot=0,ctx.rotx=(rand()%5+1);

	ctx.dx=1;
	ctx.dy=1;
	ctx.x=0;
	ctx.y=0;

	printf("Starting...\n");

	ctx.display.graphmode(vga::VGALO);
	ctx.display.cls();
	ctx.display.setpalette(palette::VGA_PAL);
	canvas::setdefpalette(ctx.display.getpalette());

	png mariopng;
	image mario;

	printf("Loading...\n");
	if (!mariopng.load("emscripten/assets/mario16.png")) {
		printf("Error loading sprite\n");
		return(1);
	}

	printf("Convert...\n");
	mariopng.convert(mario);
	printf("Free...\n");
	mariopng.free();

	printf("Math...\n");
	double sides=(double)mario.width()*(double)mario.width()+(double)mario.height()*(double)mario.height();
	int hyp=(int) sqrt(sides);
	ctx.w=hyp;
	ctx.h=hyp;

	ctx.box.size(ctx.w,ctx.h);

	printf("Building...\n");

	vtext text(ctx.display.width(),ctx.display.height(),0);
	ctx.mytext.setbg(14);
	text.drawtext(ctx.mytext,"VGA DEMO",4);

	if (!mariopng.load("emscripten/assets/mariobg.png")) {
		printf("Error loading bg\n");
		return(1);
	}

	mariopng.convert(ctx.bg);
	mariopng.free();

	ctx.box.setbg(mario.getbg());
	ctx.box.clear();
	ctx.box.copypalette(mario);
	ctx.box.drawimage((ctx.box.width()-mario.width())/2,(ctx.box.height()-mario.height())/2,mario);

	ctx.boxc=ctx.box;

#ifdef __EMSCRIPTEN__
    int simulate_infinite_loop = 1;

    emscripten_set_main_loop_arg(animate, &ctx, -1, simulate_infinite_loop);
#else
    clock_t stime,ntime;
    int hz=100;
    int clocks_per_frame=CLOCKS_PER_SEC/hz;
    int wait;
    bool endprogram=false;

    do {
		stime=clock();
		animate(&ctx);
		ntime=clock();
		wait=(clocks_per_frame-(ntime-stime))/1000;
		if (ctx.display.getEvents(wait) != 0) {
			endprogram=true;
		}
    } while(!endprogram);
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

