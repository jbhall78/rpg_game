#include "includes.h"
#include "game.h"

G_Font *
CreateFont(char *filename)
{
    G_Font *font;

    font = Malloc(sizeof(G_Font));

    if ((font->surface = SDL_LoadBMP(filename)) == NULL) {
	fprintf(stderr, "Can't load font image!\n");
	return NULL;
    }

    SDL_SetColorKey(font->surface, SDL_SRCCOLORKEY, GetPixel(font->surface, 0, 0));

    font->w = 8;
    font->h = 13;
    font->offset = 32;

    return font;
}

void
DrawText(G_Screen *screen, int x, int y, char *str)
{
    G_Font *font = screen->font;
    SDL_Rect src_area;
    SDL_Rect dst_area;
    int len = strlen(str);
    int i;

    dst_area.w = font->w;
    dst_area.h = font->h;
    dst_area.y = y;

    src_area.w = font->w;
    src_area.h = font->h;
    src_area.y = 0;
    for (i = 0; i < len; i++) {
	dst_area.x = x;

	src_area.x = (str[i] - font->offset) * font->w;

	if (SDL_BlitSurface(font->surface, &src_area, screen->surface, &dst_area) < 0) {
	    fprintf(stderr, "Error during Blit\n");
	    exit(1);
	}
	
	x += font->w;
    }
}

G_Window *
CreateWindow(G_Screen *screen, char *title, int x, int y, int w, int h)
{
    G_Window *win;
    SDL_PixelFormat *spf = screen->surface->format;
    Uint32 translucent_color, border_color, transparent_color;
    SDL_Rect area;

    translucent_color = SDL_MapRGB(spf, 50, 50, 60);
    transparent_color = SDL_MapRGB(spf, 255, 255, 255);
    border_color      = SDL_MapRGB(spf, 140, 160, 160);

    win = Malloc(sizeof(G_Window));

    win->title = title;
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;

    area.x = 0;
    area.y = 0;
    area.w = w;
    area.h = h;

    // create translucent surface
    if ((win->trans = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, spf->BitsPerPixel,
		    spf->Rmask, spf->Gmask, spf->Bmask, spf->Amask)) == NULL) {
	fprintf(stderr, "Cannot create window surface!\n");
	exit(1);
    }
    SDL_SetAlpha(win->trans, SDL_SRCALPHA, 100);

    if (SDL_FillRect(win->trans, &area, translucent_color) < 0) {
	fprintf(stderr, "Error during FillRect\n");
	exit(1);
    }

    // create border surface
    if ((win->border = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, spf->BitsPerPixel,
		    spf->Rmask, spf->Gmask, spf->Bmask, spf->Amask)) == NULL) {
	fprintf(stderr, "Cannot create border surface!\n");
	exit(1);
    }
    SDL_SetColorKey(win->border, SDL_SRCCOLORKEY, transparent_color);

    SDL_FillRect(win->border, &area, border_color);

    area.x = WIN_BORDER_THICKNESS;
    area.y = WIN_BORDER_THICKNESS;
    area.w = w - (WIN_BORDER_THICKNESS * 2);
    area.h = h - (WIN_BORDER_THICKNESS * 2);

    SDL_FillRect(win->border, &area, transparent_color);

    return win;
}

void
DrawWindow(G_Screen *screen, G_Window *win)
{
    SDL_Rect area;

    area.x = win->x;
    area.y = win->y;
    area.w = win->w;
    area.h = win->h;

    // first -- draw the transparent layer
    if (SDL_BlitSurface(win->trans, NULL, screen->surface, &area) < 0) {
	fprintf(stderr, "Error during Blit\n");
	exit(1);
    }

    // now -- draw the border
    if (SDL_BlitSurface(win->border, NULL, screen->surface, &area) < 0) {
	fprintf(stderr, "Error during Blit\n");
	exit(1);
    }
    
    // XXX - draw title
}

void
DestroyWindow(G_Window *win)
{
    SDL_FreeSurface(win->trans);
    SDL_FreeSurface(win->border);

    Free(win);
}

G_Menu *
CreateMenu(G_Screen *screen, G_MenuItem *items, char *title, int x, int y)
{
    G_Font *font = screen->font;
    G_Menu *menu;
    int i;
    int w, h;
    Uint32 color;
    SDL_PixelFormat *spf = screen->surface->format;
    SDL_Rect area;

    menu = Malloc(sizeof(G_Menu));

    w = 0;
    h = 0;
    for (i = 0; items[i].name; i++) {
	if ((strlen(items[i].name) * font->w) > w) {
	    w = strlen(items[i].name) * font->w;
	}
	h += font->h + LINE_SPACING;
    }
    menu->len = i;

    color = SDL_MapRGB(spf, 175, 0, 0);

    if ((menu->sel_surf = SDL_CreateRGBSurface(SDL_SWSURFACE,
		    w + WIN_BORDER_THICKNESS, font->h + LINE_SPACING, spf->BitsPerPixel,
                    spf->Rmask, spf->Gmask, spf->Bmask, spf->Amask)) == NULL) {
        fprintf(stderr, "Cannot create window surface!\n");
        exit(1);
    }

    area.x = 0;
    area.y = 0;
    area.w = w + WIN_BORDER_THICKNESS;
    area.h = font->h + LINE_SPACING;

    if (SDL_FillRect(menu->sel_surf, &area, color) < 0) {
        fprintf(stderr, "Error during FillRect\n");
        exit(1);
    }

    w += (MENU_X_OFFSET + WIN_BORDER_THICKNESS) * 2;
    h += (MENU_Y_OFFSET + WIN_BORDER_THICKNESS) * 2;

    menu->win = CreateWindow(screen, title, x, y, w, h);

    menu->items = items;

    menu->sel = 0;

    menu->font = font;

    return menu;
}

void
UpdateMenu(G_Menu *Menu)
{
}

void
DrawMenu(G_Screen *screen, G_Menu *menu)
{
    int i;
    int x, y;
    int h = menu->font->h;
    SDL_Rect area;

    // create the window we will be drawing to
    DrawWindow(screen, menu->win);

    for (i = 0; i < menu->len; i++) {
	if (i == menu->sel) {
	    // draw the selection surface under the text
	    area.x = menu->win->x + MENU_X_OFFSET + (WIN_BORDER_THICKNESS / 2);
	    area.y = menu->win->y + MENU_Y_OFFSET + (i * (h + LINE_SPACING)) + LINE_SPACING;
	    area.w = menu->sel_surf->w;
	    area.h = menu->sel_surf->h;


	    if (SDL_BlitSurface(menu->sel_surf, NULL, screen->surface, &area) < 0) {
		fprintf(stderr, "Error during Blit\n");
		exit(1);
	    }
	}
	x = MENU_X_OFFSET + WIN_BORDER_THICKNESS + menu->win->x;
	y = MENU_Y_OFFSET + WIN_BORDER_THICKNESS + menu->win->y + (i * (h + LINE_SPACING));

	DrawText(screen, x, y, menu->items[i].name);
    }
}

void
DestroyMenu(G_Menu *menu)
{
    DestroyWindow(menu->win);

    SDL_FreeSurface(menu->sel_surf);

    // we arent going to free up the menu items... because that data isn't copied
    // in the create function.. maybe we should? dunno

    Free(menu);
}

G_TextBox *
CreateTextBox(G_Screen *screen, char *title, char *text, int x, int y)
{
    G_Font *font = screen->font;
    G_TextBox *box;
    int w, h;

    box = Malloc(sizeof(G_TextBox));

    // need to make multiline action
    w = strlen(text) * font->w;
    h = font->h;

    w += (WIN_X_OFFSET + WIN_BORDER_THICKNESS) * 2;
    h += (WIN_Y_OFFSET + WIN_BORDER_THICKNESS) * 2;

    box->win = CreateWindow(screen, title, x, y, w, h);
    box->font = screen->font;
    box->text = text;

    return box;
}

void
DrawTextBox(G_Screen *screen, G_TextBox *box)
{
    int x, y;

    DrawWindow(screen, box->win);

    x = WIN_X_OFFSET + WIN_BORDER_THICKNESS + box->win->x;
    y = WIN_Y_OFFSET + WIN_BORDER_THICKNESS + box->win->y;

    DrawText(screen, x, y, box->text);
}

void
DestroyTextBox(G_TextBox *box)
{

}

/*
 * Menu Callbacks
 */
int
GameMenuQuitGame(G_Game *game)
{
    ScheduleShutdown(game);

    return TRUE;
}

int
GameMenuReturnToGame(G_Game *game)
{
    SetGameMode(game, MODE_PLAY);
    DestroyMenu(game->game_menu);
    game->game_menu = NULL;

    return TRUE;
}
