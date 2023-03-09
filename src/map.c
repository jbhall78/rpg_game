#include "includes.h"
#include "game.h"

G_Map *
CreateMap(G_Screen *screen)
{
    SDL_PixelFormat *spf = screen->surface->format;
    Uint32 transparent_color = SDL_MapRGB(spf, 0, 4, 56); // this must have something to do with
                                                          // using and indexed pallette
    int x, y;
    SDL_Surface *tiles[3];
    G_Map *map;

    map = Malloc(sizeof(G_Map));

    // load tiles
    if ((tiles[0] = SDL_LoadBMP(PASS_TILE)) == NULL) {
	fprintf(stderr, "Can't load tile image!\n");
	return NULL;
    }
    if ((tiles[1] = SDL_LoadBMP(NOPASS_TILE)) == NULL) {
	fprintf(stderr, "Can't load tile image!\n");
	return NULL;
    }
    SDL_SetColorKey(tiles[1], SDL_SRCCOLORKEY, GetPixel(tiles[1], 0, 0));
    if ((tiles[2] = SDL_LoadBMP(LAYER_TILE)) == NULL) {
	fprintf(stderr, "Can't load tile image!\n");
	return NULL;
    }
    SDL_SetColorKey(tiles[2], SDL_SRCCOLORKEY, transparent_color);

    map->w = MAP_WIDTH;
    map->h = MAP_HEIGHT;

    if ((map->surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		    (screen->tiles_x + 2) * TILE_WIDTH, (screen->tiles_y + 2) * TILE_HEIGHT,
		    spf->BitsPerPixel, spf->Rmask, spf->Gmask, spf->Bmask, spf->Amask)) == NULL) {
        fprintf(stderr, "Cannot create map surface!\n");
	return NULL;
    }

    // create the tiles multidimensional array
    map->tiles = Calloc(map->h, sizeof(G_Tile));
    for (y = 0; y < map->h; y++) {
	map->tiles[y] = Calloc(map->w, sizeof(G_Tile));
    }

    for (y = 0; y < map->h; y++) {
    	for (x = 0; x < map->w; x++) {
	    if ((rand() % 30) == 1) {
		map->tiles[y][x].surfaces = Calloc(2, sizeof(SDL_Surface *));
		map->tiles[y][x].n_surfaces = 2;

		map->tiles[y][x].passable = TRUE;
	    	map->tiles[y][x].surfaces[0] = tiles[0];
	    	map->tiles[y][x].surfaces[1] = tiles[2];
	    } else if ((rand() % 8) == 1) {
		map->tiles[y][x].surfaces = Calloc(2, sizeof(SDL_Surface *));
		map->tiles[y][x].n_surfaces = 2;

		map->tiles[y][x].passable = FALSE;
	    	map->tiles[y][x].surfaces[0] = tiles[0];
	    	map->tiles[y][x].surfaces[1] = tiles[1];
	    } else {
		map->tiles[y][x].surfaces = Calloc(1, sizeof(SDL_Surface *));
		map->tiles[y][x].n_surfaces = 1;

    		map->tiles[y][x].passable = TRUE;
		map->tiles[y][x].surfaces[0] = tiles[0];
	    }
	    map->tiles[y][x].occupied = FALSE;
	}
    }

    map->view_x = 0;
    map->view_y = 0;
    map->xvel = 0;
    map->yvel = 0;
    map->xmod = 0;
    map->ymod = 0;
    map->frames = 0;

    if ((map->npcs = CreateNPCs(map)) == NULL) {
        return NULL;
    }

    return map;
}

void
UpdateMap(G_Screen *screen, G_Player *player, G_Map *map)
{
    float x_step = (float)TILE_WIDTH / (float)MOVEMENT_FRAMES;
    float y_step = (float)TILE_HEIGHT / (float)MOVEMENT_FRAMES;
    int x, y;

    map->xmod = map->ymod = 0;

    // just play the scroll frames
    if (map->frames) {
	if (map->xvel) {
	    if (map->xvel > 0) {
		map->xmod = (-1 * TILE_WIDTH) + ((MOVEMENT_FRAMES - map->frames) * x_step);
	    } else if (map->xvel < 0) {
		map->xmod = (1 * TILE_WIDTH) - ((MOVEMENT_FRAMES - map->frames) * x_step);
	    }
	}
	if (map->yvel) {
	    if (map->yvel > 0) {
		map->ymod = (-1 * TILE_HEIGHT) + ((MOVEMENT_FRAMES - map->frames) * y_step);
	    } else if (map->yvel < 0) {
		map->ymod = (1 * TILE_HEIGHT) - ((MOVEMENT_FRAMES - map->frames) * y_step);
	    }
	}
	map->frames--;

	return;
    }

    map->xvel = map->yvel = 0;
    x = player->x - map->view_x;
    y = player->y - map->view_y;
    if ((x > (screen->tiles_x * 0.75)) && (map->view_x < (map->w - screen->tiles_x))) {
	map->view_x++;
	map->xvel = player->g_xvel;
	map->frames = MOVEMENT_FRAMES;
	map->xmod = -1 * (map->xvel * TILE_WIDTH);
    } else if ((x < (screen->tiles_x * 0.25)) && (map->view_x > 0)) {
	map->view_x--;
	map->xvel = player->g_xvel;
	map->frames = MOVEMENT_FRAMES;
	map->xmod = -1 * (map->xvel * TILE_WIDTH);
    }
    if ((y > (screen->tiles_y * 0.75)) && (map->view_y < (map->h - screen->tiles_y))) {
	map->view_y++;
	map->yvel = player->g_yvel;
	map->frames = MOVEMENT_FRAMES;
	map->ymod = -1 * (map->yvel * TILE_HEIGHT);
    } else if ((y < (screen->tiles_y * 0.25)) && (map->view_y > 0)) {
	map->view_y--;
	map->yvel = player->g_yvel;
	map->frames = MOVEMENT_FRAMES;
	map->ymod = -1 * (map->yvel * TILE_HEIGHT);
    }
}

void
DrawMap(G_Screen *screen, G_Player *player, G_Map *map)
{
    SDL_Rect src_area, dst_area;
    int l, x, y;
    int map_x, map_y;
    int ex = screen->tiles_x + 2;
    int ey = screen->tiles_y + 2;

    // blackout the map surface 
    dst_area.x = 0;
    dst_area.y = 0;
    dst_area.w = map->surface->w;
    dst_area.h = map->surface->h;
    SDL_FillRect(map->surface, &dst_area, 0);

    // instead of `3' i need a max_surfaces variable or something
    for (l = 0; l < 3; l++) {
	for (y = 0; y < ey; y++) {
	    for (x = 0; x < ex; x++) {
		map_y = y + (map->view_y - 1);
	    	map_x = x + (map->view_x - 1);

                if ((map_x < 0) || (map_x >= map->w) || (map_y < 0) || (map_y >= map->h)) {
                    continue;
                }

		if ((l < map->tiles[map_y][map_x].n_surfaces) &&
			(map->tiles[map_y][map_x].surfaces[l] != NULL)) {
	    
		    dst_area.w = TILE_WIDTH;
		    dst_area.h = TILE_HEIGHT;
		    dst_area.x = x * TILE_WIDTH;
		    dst_area.y = y * TILE_HEIGHT;
	    
		    SDL_BlitSurface(map->tiles[map_y][map_x].surfaces[l], NULL,
			    map->surface, &dst_area);
		}
	    }
	}

	if (l == player->layer) {
	    // draw player
	    DrawPlayer(map->surface, map, player);
	}

	DrawNPCs(map->surface, map, l);
    }

    // now draw the map surface buffer to the screen with our scroll offset
    src_area.x = TILE_WIDTH + map->xmod;
    src_area.y = TILE_HEIGHT + map->ymod;
    src_area.w = screen->w;
    src_area.h = screen->h;

    dst_area.x = 0;
    dst_area.y = 0;
    dst_area.w = screen->w;
    dst_area.h = screen->h;

    SDL_BlitSurface(map->surface, &src_area, screen->surface, &dst_area);
}

int
CanMoveTo(G_Map *map, int x, int y)
{
    if ((x >= map->w) || (x < 0))
	return FALSE;
    if ((y >= map->h) || (y < 0)) 
	return FALSE;
    
    if (map->tiles[y][x].passable != TRUE)
	return FALSE;
    
    if (map->tiles[y][x].occupied != FALSE)
	return FALSE;
    
    return TRUE;
}

int
GetRandomDirection(G_Map *map, int x, int y)
{
    int used_directions[NUM_DIRECTIONS];
    int directions_full = FALSE;
    int i;
    int tx, ty;
    int xvel, yvel;
    int dir;
    
    for (i = 0; i < NUM_DIRECTIONS; i++) {
        used_directions[i] = FALSE;
    }

    while (directions_full == FALSE) {
       	dir = rand() % NUM_DIRECTIONS;
       	used_directions[dir] = TRUE;
	
	GetVelocitiesFromDirection(dir, &xvel, &yvel);
	
	tx = x + xvel;
	ty = y + yvel;
	
	if (CanMoveTo(map, tx, ty)) {
	    break;
	}

	// now check to see if we have moved in every direction
	directions_full = TRUE;
	for (i = 0; i < NUM_DIRECTIONS; i++) {
	    if (used_directions[i] == FALSE) {
		directions_full = FALSE;
	    }
	}
    }
    
    if (directions_full)
	return -1;
    
    return dir;
}

void
GetRandomMapPosition(G_Map *map, int *px, int *py)
{
    int x;
    int y;
    int valid_cords = FALSE;

    while (valid_cords != TRUE) {
        x = rand() % MAP_WIDTH;
        y = rand() % MAP_HEIGHT;

        if (CanMoveTo(map, x, y))
            valid_cords = TRUE;
    }

    *px = x;
    *py = y;
}

int
IsOccupied(G_Map *map, int x, int y)
{
    int i;
    G_NPC *npcs = map->npcs;

    if ((x < 0) || (x >= map->w)) 
	return 0;
    if ((y < 0) || (y >= map->h)) 
	return 0;

    if (map->tiles[y][x].occupied) {
	for (i = 0; i < NUM_NPCS; i++) {
	    if ((npcs[i].x == x) && (npcs[i].y == y)) {
		return i;
	    }
	}

	return -1; // must be a player
    }

    return 0;
}
