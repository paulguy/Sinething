#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <SDL.h>

#include "btext.h"

#define BOUND_LEN		(2.0)
#define DEFORM_LEN		(1.1)
#define BOUND_AMP		(0.1)
#define DEFORM_AMP		(0.2)
#define BOUND_PHASE		(M_PI / 4.0)
#define BOUND_DIFF		(0.2)
#define BOUND_SPEED		(1)
#define DEFORM_SPEED	(3)
#define TEXT_SPEED		(7)

#define CHAR_PIXEL		(16)

#define FPS				(60)
#define DEFAULT_SIZE	(512)
#define FRAMELEN		(1000/FPS)
#define STRIP_WIDTH		(1)


#define SETPIXEL(S, X, Y, C)	((Uint32 *)S->pixels)[(S->w * Y) + X] = C

const char const* scrolltext = "The quick brown fox jumps over the lazy dog.      ";

typedef struct {
	unsigned int boundlen;
	unsigned int deformlen;
	unsigned int *topbound;
	unsigned int *btmbound;
	int *deform;
} trigtbl;

trigtbl *trigtbl_init(unsigned int width, unsigned int height);
void trigtbl_free(trigtbl *t);
void setpixel(SDL_Surface *surface, unsigned int x, unsigned int y, Uint32 color);

int main(int argc, char **argv) {
	SDL_Surface *screen;
	SDL_Event event;
	trigtbl *t;
	int running;
	unsigned int framestart, framelen;
	Uint32 white, black;
	SDL_Rect allscreen;
	SDL_Rect charstrip;
	bfont *font;
	unsigned int *textposes;
	unsigned int textlen;
	unsigned int textwidth;
	unsigned int charpxlphase, charpxlphase2;
	unsigned int charpxl, charpxl2;
	unsigned int curchar, curchar2;
	unsigned int curwidth, curwidth2;
	unsigned int boundphase;
	unsigned int deformphase;
	unsigned int stripheight;
	unsigned int i;

	charstrip.w = STRIP_WIDTH;

	allscreen.x = 0;
	allscreen.y = 0;
	allscreen.w = DEFAULT_SIZE;
	if(argc > 1)
		allscreen.w = atoi(argv[1]);

	if(argc > 2)
		allscreen.h = atoi(argv[2]);
	else
		allscreen.h = allscreen.w;

	t = trigtbl_init(allscreen.w, allscreen.h);
	if(t == NULL) {
		fprintf(stderr, "Failed to create trig table.\n");
		goto error0;
	}

/*	fprintf(stderr, "%u %u\n", t->boundlen, t->deformlen);
	for(i = 0; i < t->boundlen; i++)
		fprintf(stderr, "%u ", t->topbound[i]);
	fprintf(stderr, "\n");
	for(i = 0; i < t->boundlen; i++)
		fprintf(stderr, "%u ", t->btmbound[i]);
	fprintf(stderr, "\n");
	for(i = 0; i < t->deformlen; i++)
		fprintf(stderr, "%i ", t->deform[i]);
	fprintf(stderr, "\n");
*/
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL.\n");
		goto error1;
	}

	screen = SDL_SetVideoMode(allscreen.w, allscreen.h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if(screen == NULL) {
		fprintf(stderr, "Couldn't set video mode %ux%u.\n", allscreen.w, allscreen.h);
		goto error2;
	}

	font = btext_loadFromBMP("font.bmp");
	if(font == NULL) {
		fprintf(stderr, "Failed to load font font.bmp.\n");
		goto error2;
	}

	white = SDL_MapRGB(screen->format, 255, 255, 255);
	black = SDL_MapRGB(screen->format, 0, 0, 0);

	textwidth = btext_calcWidth(font, scrolltext);
	textlen = strlen(scrolltext);
	textposes = malloc(sizeof(unsigned int) * textlen);
	if(textposes == NULL) {
		perror("malloc");
		goto error3;
	}
	btext_charPoses(font, scrolltext, textposes);

	charpxlphase = 0;
	charpxl = 0;
	curchar = 0;
	curwidth = btext_getWidth(font, scrolltext[curchar]);
	boundphase = 0;
	deformphase = 0;
	running = 1;
	while(running) {
		framestart = SDL_GetTicks();
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				case SDL_QUIT:
					running = 0;
				default:
					break;
			}
		}

		boundphase += BOUND_SPEED;
		if(boundphase >= t->boundlen)
			boundphase %= t->boundlen;
		deformphase += DEFORM_SPEED;
		if(deformphase >= t->deformlen)
			deformphase %= t->deformlen;
		charpxlphase += TEXT_SPEED;
		if(charpxlphase >= CHAR_PIXEL) {
			charpxl += charpxlphase / CHAR_PIXEL;
			charpxlphase %= CHAR_PIXEL;
			if(charpxl > textwidth) {
				charpxl %= textwidth;
				curchar = 0;
				curwidth = btext_getWidth(font, scrolltext[curchar]);
			}
			if(charpxl > textposes[curchar] + curwidth) {
				curchar++;
				curwidth = btext_getWidth(font, scrolltext[curchar]);
			}
		}
		SDL_FillRect(screen, &allscreen, black);
//		if(SDL_LockSurface(screen) == -1)
//			goto error2;

		curchar2 = curchar;
		charpxlphase2 = charpxlphase;
		charpxl2 = charpxl;
		curwidth2 = curwidth;
		for(charstrip.x = 0; charstrip.x < allscreen.w; charstrip.x+=STRIP_WIDTH) {
			unsigned int yb = (boundphase + charstrip.x) % t->boundlen;
			unsigned int yd = (deformphase + charstrip.x) % t->deformlen;
			charpxlphase2+=STRIP_WIDTH;
			if(charpxlphase2 >= CHAR_PIXEL) {
				charpxl2 += charpxlphase2 / CHAR_PIXEL;
				charpxlphase2 %= CHAR_PIXEL;
				if(charpxl2 > textwidth) {
					charpxl2 %= textwidth;
					curchar2 = 0;
					curwidth2 = btext_getWidth(font, scrolltext[curchar2]);
				}
				if(charpxl2 > textposes[curchar2] + curwidth2) {
					curchar2++;
					curwidth2 = btext_getWidth(font, scrolltext[curchar2]);
				}
			}
//			setpixel(screen, i, t->topbound[yb] + t->deform[yd], white);
//			setpixel(screen, i, t->btmbound[yb] + t->deform[yd], white);
			stripheight = t->btmbound[yb] - t->topbound[yb];
			charstrip.h = stripheight / font->height;
			charstrip.h += !!(stripheight % font->height);
			for(i = 0; i < font->height; i++) {
				charstrip.y = t->topbound[yb] + t->deform[yd] + (stripheight * i / font->height);
				if(btext_pixelAt(font, scrolltext[curchar2], charpxl2 - textposes[curchar2] - 1, i))
					SDL_FillRect(screen, &charstrip, white);
			}
		}

//		SDL_UnlockSurface(screen);
		SDL_Flip(screen);

		framelen = SDL_GetTicks() - framestart;
//		fprintf(stderr, "%u ", framelen);
		unsigned int waittime = FRAMELEN - framelen;
		if(waittime < FRAMELEN && waittime > 0)
			SDL_Delay(FRAMELEN - framelen);
//		running = 0; // just do 1 frame
	}

	free(textposes);
	btext_free(font);
	trigtbl_free(t);
	SDL_Quit();
	exit(EXIT_SUCCESS);


	error3:
	btext_free(font);
	error2:
	SDL_Quit();
	error1:
	trigtbl_free(t);
	error0:
	exit(EXIT_FAILURE);
}

trigtbl *trigtbl_init(unsigned int width, unsigned int height) {
	unsigned int i;
	trigtbl *t = malloc(sizeof(trigtbl));

	t->boundlen = (unsigned int)((float)width * BOUND_LEN);
	t->deformlen = (unsigned int)((float)width * DEFORM_LEN);

	if(t == NULL)
		goto error1;
	t->topbound = malloc(sizeof(unsigned int) * t->boundlen);
	if(t->topbound == NULL)
		goto error2;
	t->btmbound = malloc(sizeof(unsigned int) * t->boundlen);
	if(t->btmbound == NULL)
		goto error3;
	t->deform = malloc(sizeof(unsigned int) * t->deformlen);
	if(t->deform == NULL)
		goto error4;

	for(i = 0; i < t->boundlen; i++) {
		t->topbound[i] = (unsigned int)((double)(sin(M_PI * 2.0 / (double)t->boundlen * (double)i)
		                                * BOUND_AMP - BOUND_DIFF + 0.5) * (double)height);
		t->btmbound[i] = (unsigned int)((double)(sin(M_PI * 2.0 / (double)t->boundlen * (double)i + BOUND_PHASE)
		                                * BOUND_AMP + BOUND_DIFF + 0.5) * (double)height);
	}
	for(i = 0; i < t->deformlen; i++)
		t->deform[i] =   (int)((double)(sin(M_PI * 2.0 / (double)t->deformlen * (double)i)
		                                * DEFORM_AMP) * (double)height);

	return(t);

	error4:
	free(t->btmbound);
	error3:
	free(t->topbound);
	error2:
	free(t);
	error1:
	return(NULL);
}

void trigtbl_free(trigtbl *t) {
	free(t->topbound);
	free(t->btmbound);
	free(t->deform);
	free(t);
}

void setpixel(SDL_Surface *surface, unsigned int x, unsigned int y, Uint32 color) {
	((Uint32 *)surface->pixels)[(surface->w * y) + x] = color;
}
