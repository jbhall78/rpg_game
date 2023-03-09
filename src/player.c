#include "includes.h"
#include "game.h"

G_Player *
CreatePlayer(G_Screen *screen, G_Map *map)
{
    G_Player *player;

    player = Malloc(sizeof(G_Player));

    // place the player in center of map
    player->x = map->w / 2;
    player->y = map->h / 2;
    player->layer = 0;
    map->tiles[player->y][player->x].occupied = TRUE;

    player->xvel = VEL_STOP;
    player->yvel = VEL_STOP;
    if ((player->surface = SDL_LoadBMP(PLAYER)) == NULL) {
	fprintf(stderr, "Can't load player image!\n");
	return NULL;
    }

    SDL_SetColorKey(player->surface, SDL_SRCCOLORKEY, GetPixel(player->surface, 0, 0));

    // set the map viewport for the current view (place player in center of view)
    map->view_x = (map->w / 2) - (screen->tiles_x / 2);
    map->view_y = (map->h / 2) - (screen->tiles_y / 2);

    return player;
}

void
UpdatePlayer(G_Game *game)
{
    G_Map *map = game->map;
    G_Player *player = game->player;
    int new_x, new_y;
    int go_x, go_y;
    int id;
    float x_step = (float)TILE_WIDTH / (float)MOVEMENT_FRAMES;
    float y_step = (float)TILE_HEIGHT / (float)MOVEMENT_FRAMES;
    int frame_frames = MOVEMENT_FRAMES / PLAYER_FRAMES; // number of frames a particular frame
                                                        // appears on the screen
                                                    
    player->xmod = player->ymod = 0;

    // we are supposed to play our movement frames to the next block, not update our position.
    if (player->frames) {
	if (player->g_xvel) {
	    if (player->g_xvel > 0) {
		player->xmod = (-1 * TILE_WIDTH) + ((MOVEMENT_FRAMES - player->frames) * x_step);
	    } else if (player->g_xvel < 0) {
		player->xmod = (1 * TILE_WIDTH) - ((MOVEMENT_FRAMES - player->frames) * x_step);
	    }
	}
	if (player->g_yvel) {
	    if (player->g_yvel > 0) {
		player->ymod = (-1 * TILE_HEIGHT) + ((MOVEMENT_FRAMES - player->frames) * y_step);
	    } else if (player->g_yvel < 0) {
		player->ymod = (1 * TILE_HEIGHT) - ((MOVEMENT_FRAMES - player->frames) * y_step);
	    }
	}

	player->frames--;
	
	// now update the animation frame
	if (! player->change_time) {
	    if (++player->cur_frame >= PLAYER_FRAMES) {
		player->cur_frame = 0;
	    }
	    player->change_time = frame_frames;
	} else {
	    player->change_time--;
	}

	return;
    }
    player->cur_frame = 0;
    player->change_time = 0;

    if ((! player->xvel) && (! player->yvel)) {
	return;
    }

    go_x = TRUE;
    go_y = TRUE;

    new_x = player->x + player->xvel;
    new_y = player->y + player->yvel;

    if (player->xvel != 0) {
	if (! CanMoveTo(map, new_x, player->y)) {
	    if ((id = IsOccupied(map, new_x, player->y)) > 0) {
		if (InitiateNPCContact(game, id) == TRUE)
		    return;
	    }
	    go_x = FALSE;
	}
    }
    if (player->yvel != 0) {
	if (! CanMoveTo(map, player->x, new_y)) {
	    if ((id = IsOccupied(map, player->x, new_y)) > 0) {
		if (InitiateNPCContact(game, id) == TRUE)
		    return;
	    }
	    go_y = FALSE;
	}
    }

    // we are either trying to move diagonally or we are stuck up against something
    // and still want to move.
    if ((player->xvel != 0) && (player->yvel != 0)) {
	if (! CanMoveTo(map, new_x, new_y)) {
	    if ((id = IsOccupied(map, new_x, new_y)) > 0) {
		if (InitiateNPCContact(game, id) == TRUE)
		    return;
	    }
	    go_x = FALSE;
	    go_y = FALSE;
	}

	// if we can move in a different direction, lets do it.
	if (CanMoveTo(map, player->x, new_y)) {
	    go_y = TRUE;
	}
	if (CanMoveTo(map, new_x, player->y)) {
	    go_x = TRUE;
	}
	if (go_y && go_x && (! CanMoveTo(map, new_x, new_y))) {
	    go_y = go_x = FALSE;
	}
    }
    
    player->g_xvel = 0;
    player->g_yvel = 0;
    if (go_x || go_y) {
	map->tiles[player->y][player->x].occupied = FALSE;
	if (go_x) {
	    if (player->x != new_x) {
		player->x = new_x;
		player->frames = MOVEMENT_FRAMES;
	    }
	    player->g_xvel = player->xvel;
	}
	if (go_y) {
	    if (player->y != new_y) {
		player->y = new_y;
		player->frames = MOVEMENT_FRAMES;
	    }
	    player->g_yvel = player->yvel;
	} 
	player->xmod = -1 * (player->g_xvel * TILE_WIDTH);
	player->ymod = -1 * (player->g_yvel * TILE_HEIGHT);

	map->tiles[player->y][player->x].occupied = TRUE;
    } 

    // Set the player direction
    player->dir = GetDirectionFromVelocities(player->xvel, player->yvel);
}

void
DrawPlayer(SDL_Surface *surface, G_Map *map, G_Player *player)
{
    SDL_Rect dst_area;
    SDL_Rect src_area;
    int dir = player->dir;

    if ((dir == DIR_NORTHWEST) || (dir == DIR_NORTHEAST))
	dir = DIR_NORTH;
    if ((dir == DIR_SOUTHEAST) || (dir == DIR_SOUTHWEST))
	dir = DIR_SOUTH;

    src_area.w = TILE_WIDTH;
    src_area.h = TILE_HEIGHT;
    src_area.x = player->cur_frame * TILE_WIDTH;
    src_area.y = dir * TILE_HEIGHT;

    dst_area.w = TILE_WIDTH;
    dst_area.h = TILE_HEIGHT;

    dst_area.x = ((player->x - (map->view_x - 1)) * TILE_WIDTH) + player->xmod;
    dst_area.y = ((player->y - (map->view_y - 1)) * TILE_HEIGHT) + player->ymod;

    SDL_BlitSurface(player->surface, &src_area, surface, &dst_area);
}

void
PlayerUse(G_Game *game)
{
    G_Player *player = game->player;
    G_Map *map = game->map;
    int xvel, yvel;
    int x, y;
    int id;

    if (game->script != NULL)
	return;

    GetVelocitiesFromDirection(player->dir, &xvel, &yvel);

    x = player->x + xvel;
    y = player->y + yvel;

    if ((id = IsOccupied(map, x, y)) > 0) {
	InitiateNPCUse(game, id);

	return;
    } 
}
