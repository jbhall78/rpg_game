#include "includes.h"
#include "game.h"

G_Screen *
CreateScreen(int w, int h, int bpp, Uint32 flags)
{
    G_Screen *screen;

    screen = Malloc(sizeof(G_Screen));

    screen->bpp = bpp;
    screen->frames = 1;
    screen->w = w;
    screen->h = h;
    screen->tiles_x = w / TILE_WIDTH;
    screen->tiles_y = h / TILE_HEIGHT;

    // initialize SDL and create our screen
    // if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
	return NULL;
    }
    if ((screen->surface = SDL_SetVideoMode(w, h, bpp, flags)) == NULL) {
	fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
	return NULL;
    }
    SDL_WM_SetCaption("Concept Demo", NULL);
    SDL_ShowCursor(SDL_DISABLE);

    SDL_SetAlpha(screen->surface, SDL_RLEACCEL, 0);

    // now load our font
    if ((screen->font = CreateFont(FONT)) == NULL) {
        return NULL;
    }
    
    return screen;
}

void
UpdateFPS(G_Screen *screen)
{
    screen->frames++;
}

void
DrawFPS(G_Screen *screen)
{
    char buf[1024];
    int seconds = SDL_GetTicks() / 1000;

    if (! seconds)
        seconds = 1;

    snprintf(buf, 1024, "Frames Per Second: %d", screen->frames / seconds);
    DrawText(screen, 10, 10, buf);
}

void
DrawPixel(SDL_Surface *surface, int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
    Uint32 color = SDL_MapRGB(surface->format, R, G, B);

    if (SDL_MUSTLOCK(surface)) {
	if (SDL_LockSurface(surface) < 0) {
	    return;
	}
    }
    switch (surface->format->BytesPerPixel) {
	case 1: { /* Assuming 8-bpp */
    		Uint8 *bufp;

		bufp = (Uint8 *)surface->pixels + y*surface->pitch + x;
		*bufp = color;
	    }
	    break;

	case 2: { /* Probably 15-bpp or 16-bpp */
    		Uint16 *bufp;

		bufp = (Uint16 *)surface->pixels + y*surface->pitch/2 + x;
		*bufp = color;
	    }
	    break;

	case 3: { /* Slow 24-bpp mode, usually not used */
    		Uint8 *bufp;

		bufp = (Uint8 *)surface->pixels + y*surface->pitch + x * 3;
		if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
		    bufp[0] = color;
		    bufp[1] = color >> 8;
		    bufp[2] = color >> 16;
		} else {
		    bufp[2] = color;
		    bufp[1] = color >> 8;
		    bufp[0] = color >> 16;
		}
	    }
	    break;

	case 4: { /* Probably 32-bpp */
    		Uint32 *bufp;
		    
		bufp = (Uint32 *)surface->pixels + y*surface->pitch/4 + x;
		*bufp = color;
	    }
	    break;
    }
    if (SDL_MUSTLOCK(surface)) {
	SDL_UnlockSurface(surface);
    }
    SDL_UpdateRect(surface, x, y, 1, 1);
}

Uint32
GetPixel(SDL_Surface *surface, int x, int y)
{
    int bpp;
    Uint8 *p;
    Uint32 retval;

    if (SDL_MUSTLOCK(surface)) {
      	if (SDL_LockSurface(surface) < 0) {
	    return 0;
	}
    }

    bpp = surface->format->BytesPerPixel;
    p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
	case 1:
	    retval = *p;
	    break;

	case 2:
	    retval = *(Uint16 *)p;
	    break;
	    
	case 3:
	    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		retval = p[0] << 16 | p[1] << 8 | p[2];
	    else
		retval = p[0] | p[1] << 8 | p[2] << 16;
	    break;

	case 4:
	    retval = *(Uint32 *)p;
	    break;

	default:
	    retval = 0;
	    break;       /* shouldn't happen, but avoids warnings */
    }

    if (SDL_MUSTLOCK(surface)) {
       	SDL_UnlockSurface(surface);
    }

    return retval;
}
