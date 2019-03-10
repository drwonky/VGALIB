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
	image score;
	image playfield;
	image ball;
	image ballr;
	vga display;
	unsigned int x,y,w,h,dx,dy;
	int rot,rotframes;
} context;


void animate(void *arg)
{
		context *ctx=(context *)arg;

		ctx->display.screen.drawimage(0,0,ctx->score);
		ctx->display.screen.drawimage(0,20,ctx->playfield);
		ctx->ball.rotate(ctx->ballr,ctx->rot);
		ctx->display.screen.drawsprite(ctx->x,ctx->y,ctx->ballr);

		ctx->display.update();
		ctx->rotframes++;
		if(ctx->rotframes%5 == 0)
			ctx->rot+=45;
		if(ctx->rot>360) ctx->rot=0;

		ctx->x+=ctx->dx;
		ctx->y+=ctx->dy;

		if(ctx->x>ctx->display.width()-ctx->w-1 || ctx->x<1) {
			ctx->dx*=-1;
		}

		if(ctx->y>ctx->display.height()-ctx->h-1 || ctx->y<20) {
			ctx->dy*=-1;
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

	ctx.rot=0,ctx.rotframes=0;

	ctx.dx=1;
	ctx.dy=1;
	ctx.x=0;
	ctx.y=20;

	printf("Starting...\n");

	ctx.display.graphmode(vga::X16);
	ctx.display.cls();
	ctx.display.setpalette(palette::CGA_PAL);
	canvas::setdefpalette(ctx.display.getpalette());

	printf("Loading...\n");
	png asset;

	printf("Loading assets...\n");
	if (!(asset.loadimage("emscripten/assets/score.png",ctx.score) &&
		asset.loadimage("emscripten/assets/playfield.png",ctx.playfield) &&
		asset.loadimage("emscripten/assets/ball.png",ctx.ball))) {
		printf("Error loading sprite\n");
		return(1);
	}

	asset.free();

	ctx.w=ctx.ball.width();
	ctx.h=ctx.ball.height();

	ctx.ballr=ctx.ball;

#ifdef __EMSCRIPTEN__
    int simulate_infinite_loop = 1;

    emscripten_set_main_loop_arg(animate, &ctx, -1, simulate_infinite_loop);
#else
    uint32_t start_time,end_time,sched_time;
    int hz=60;		// Target event rate, vsync is what actually controls our speed, this delay is just how long we wait for events, ensuring the render loop always has priority
    int clocks_per_frame=1000/hz;
    int wait;
    bool endprogram=false;

    ctx.display.getEvents(20);  // debounce

    do {
		start_time=SDL_GetTicks();
		sched_time=start_time+clocks_per_frame;
		animate(&ctx);
		end_time=SDL_GetTicks();
//		wait=(clocks_per_frame-(ntime-stime))/1000;	// generate accurate target times in the future
		wait=(sched_time - end_time);
		wait=wait<0?0:wait;							// If render took too long it will result in negative wait.
//		cout <<"wait delay "<<wait<<" start "<<start_time<<" sched "<<sched_time<<" end "<<end_time<<" clocks "<<clocks_per_frame<<endl;
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

