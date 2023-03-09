#include "includes.h"
#include "game.h"

/*
 * Global Variables
 */
G_MenuItem GameMenuItems[] = {
    { SDLK_i,       "Inventory",      (G_Function *)NULL   },
    { SDLK_p,       "Player Info",    (G_Function *)NULL   },
    { SDLK_s,       "Save Game",      (G_Function *)NULL   },
    { SDLK_l,       "Load Game",      (G_Function *)NULL   },
    { SDLK_q,       "Quit Game",      GameMenuQuitGame     }, 
    { SDLK_r,       "Return to Game", GameMenuReturnToGame }, 
    { SDLK_UNKNOWN, NULL,             (G_Function *)NULL   }
};

int
main(int argc, char **argv)
{
    Uint32 next_time;
    G_Game *game;

    srand(1337);

    if ((game = CreateGame(argc, argv)) == NULL) {
	fprintf(stderr, "Error: Can't Initialize Game!\n");
	exit(1);
    }

    next_time = SDL_GetTicks() + TICK_INTERVAL;
    while (ShutdownScheduled(game) != TRUE) {
	if (! TimeLeft(next_time)) {
	    ProcessEvents(game);

	    UpdateGame(game);

    	    next_time += TICK_INTERVAL;
	}

	DrawGame(game);

	// update the display
	SDL_Flip(game->screen->surface);

	UpdateFPS(game->screen);
    }

    SDL_Quit();

    return 0;
}

G_Game *
CreateGame(int argc, char **argv)
{
    G_Game *game;
    Uint32 flags;

    // allocate the structure
    game = Malloc(sizeof(G_Game));

    game->game_menu = NULL;
    game->shutdown = FALSE;
    game->npc_id = -1;

    game->up_down = FALSE;
    game->down_down = FALSE;
    game->left_down = FALSE;
    game->right_down = FALSE;

    flags = SDL_HWSURFACE | SDL_DOUBLEBUF;

    // see if we need to be in fullscreen mode
    while ( argc > 1 ) {
    	--argc;
	if ( strcmp(argv[argc], "-fullscreen") == 0 ) {
	    flags |= SDL_FULLSCREEN;
	} else {
	    fprintf(stderr, "Usage: %s [-fullscreen]\n", argv[0]);
	    exit(1);
	}
    }

    // create screen
    if ((game->screen = CreateScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 16, flags)) == NULL) {
	return NULL;
    }

    // generate map
    if ((game->map = CreateMap(game->screen)) == NULL) {
	return NULL;
    }

    // initialize player
    if ((game->player = CreatePlayer(game->screen, game->map)) == NULL) {
	return NULL;
    }

    SetGameMode(game, MODE_PLAY);

    return game;
}

void
UpdateGame(G_Game *game)
{
    int mode = GetGameMode(game);

    if (mode == MODE_PLAY) {
	UpdatePlayer(game);

	UpdateNPCs(game->map);

	UpdateMap(game->screen, game->player, game->map);
    } else if (mode == MODE_GAMEMENU) {
	UpdateMenu(game->game_menu);
    } else if (mode == MODE_NPC_INTERACTION) {
	UpdateScript();
    }
}

void
DrawGame(G_Game *game)
{
    int mode = GetGameMode(game);

    DrawMap(game->screen, game->player, game->map); // draws the map tiles

    if (mode == MODE_GAMEMENU) {
	DrawMenu(game->screen, game->game_menu);
    } else if (mode == MODE_NPC_INTERACTION) {
	DrawScript(game);
    }

    DrawFPS(game->screen);
}

int
GetGameMode(G_Game *game)
{
    return game->mode;
}

void
SetGameMode(G_Game *game, int mode)
{
    game->mode = mode;
}

void
ScheduleShutdown(G_Game *game)
{   
    game->shutdown = TRUE;
}

int
ShutdownScheduled(G_Game *game)
{
    return game->shutdown;
}

void
ProcessEvents(G_Game *game)
{
    SDL_Event event;
    int mode;
    G_Player *player = game->player;


    // poll for events
    while (SDL_PollEvent(&event)) {
 	mode = GetGameMode(game);

	// global key bindings
	switch(event.type) {
    	    case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		    case SDLK_q:
			ScheduleShutdown(game);
			break;
		    default:
			break;
		}
		break;
	    case SDL_QUIT:
		ScheduleShutdown(game);
		break;
	    default:
		break;
	}

	// mode specific key bindings
	if (mode == MODE_PLAY) {

	    // main gameplay bindings
	    switch(event.type) {
		case SDL_KEYDOWN:
		    switch (event.key.keysym.sym) {
			case SDLK_LEFT:
			    if (! player->xvel) {
				player->xvel = VEL_LEFT;
			    }
			    game->left_down = TRUE;
			    break;
			case SDLK_RIGHT:
			    if (! player->xvel) {
				player->xvel = VEL_RIGHT;
			    }
			    game->right_down = TRUE;
			    break;
			case SDLK_UP:
			    if (! player->yvel) {
				player->yvel = VEL_UP;
			    }
			    game->up_down = TRUE;
			    break;
			case SDLK_DOWN:
			    if (! player->yvel) {
				player->yvel = VEL_DOWN;
			    }
			    game->down_down = TRUE;
			    break;
			case SDLK_ESCAPE:
			    SetGameMode(game, MODE_GAMEMENU);

			    player->xvel = VEL_STOP;
			    player->yvel = VEL_STOP;
			    game->left_down  = FALSE;
			    game->right_down = FALSE;
			    game->up_down    = FALSE;
			    game->down_down  = FALSE;

			    game->game_menu = CreateMenu(game->screen, GameMenuItems,
				    "Game Menu", 30, 30);
			    break;
			case SDLK_SPACE:
			    PlayerUse(game);
			    break;
			default:
			    break;
		    }
		    break;
		case SDL_KEYUP:
		    switch (event.key.keysym.sym) {
			case SDLK_LEFT:
			    if (game->right_down) {
				player->xvel = VEL_RIGHT;
			    } else {
				player->xvel = VEL_STOP;
			    }
			    game->left_down = FALSE;
			    break;
			case SDLK_RIGHT:
			    if (game->left_down) {
				player->xvel = VEL_LEFT;
			    } else {
				player->xvel = VEL_STOP;
			    }
			    game->right_down = FALSE;
			    break;
			case SDLK_UP:
			    if (game->down_down) {
				player->yvel = VEL_DOWN;
			    } else {
				player->yvel = VEL_STOP;
			    }
			    game->up_down = FALSE;
			    break;
			case SDLK_DOWN:
			    if (game->up_down) {
				player->yvel = VEL_UP;
			    } else {
				player->yvel = VEL_STOP;
			    }
			    game->down_down = FALSE;
			    break;
			default:
			    break;
		    }
		    break;
		default:
		    break;
	    }

	} else if (mode == MODE_GAMEMENU) {
	    G_Menu *menu = game->game_menu;

	    // Menu Key Bindings
	    switch(event.type) {
		case SDL_KEYDOWN:
		    switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
			    GameMenuReturnToGame(game);
			    break;
			case SDLK_UP:
			    if (menu->sel > 0)
				menu->sel -= 1;
			    else
				menu->sel = (menu->len - 1);
			    break;
			case SDLK_DOWN:
			    if (menu->sel < (menu->len - 1))
				menu->sel += 1;
			    else
				menu->sel = 0;
			    break;
			case SDLK_RETURN:
			case SDLK_SPACE:
			    {
				int s         = menu->sel;
				G_Function *f = menu->items[s].function;

				if (f) {
				    f(game);
				}
			    }
			    break;
			default:
			    break;
		    }
		    break;
		default:
		    break;
	    }

	} else if (mode == MODE_NPC_INTERACTION) {
	    G_Menu *menu = game->script->menu;

            switch(event.type) {
                case SDL_KEYDOWN:
		    if (game->script->type == SCRIPT_PLAYER_MENU) {
	    		switch (event.key.keysym.sym) {
    			    case SDLK_UP:
				if (menu->sel > 0)
				    menu->sel -= 1;
				else
				    menu->sel = (menu->len - 1);
				break;
			    case SDLK_DOWN:
				if (menu->sel < (menu->len - 1))
				    menu->sel += 1;
				else
				    menu->sel = 0;
				break;
			    case SDLK_RETURN:
			    case SDLK_SPACE:
				StepScript(game, menu->sel);
			    	break;
			    default:
				break;
			}
		    } else {
	    		switch (event.key.keysym.sym) {
			    case SDLK_SPACE:
				StepScript(game, 0);
				    
				break;
			    default:
				break;
			}
		    }
		default:
		    break;
	    }

	}
    }

    return;
}

int
GetDirectionFromVelocities(int xvel, int yvel)
{
    int dir;

    dir = DIR_NONE;

    if ((xvel < 0) && (yvel == 0))
	dir = DIR_WEST;
    else if ((xvel > 0) && (yvel == 0))
	dir = DIR_EAST;
    else if ((xvel == 0) && (yvel < 0))
	dir = DIR_NORTH;
    else if ((xvel == 0) && (yvel > 0))
	dir = DIR_SOUTH; 
    else if ((xvel > 0) && (yvel > 0))
	dir = DIR_SOUTHEAST;
    else if ((xvel > 0) && (yvel < 0))
	dir = DIR_NORTHEAST;
    else if ((xvel < 0) && (yvel > 0)) 
	dir = DIR_SOUTHWEST;
    else if ((xvel < 0) && (yvel < 0))
	dir = DIR_NORTHWEST;

    return dir;
}

void
GetVelocitiesFromDirection(int dir, int *xvel, int *yvel)
{
    int n_xvel, n_yvel;

    n_xvel = n_yvel = 0;

    switch(dir) {
	case DIR_NORTH:
	    n_yvel--;
	    break;
	case DIR_SOUTH:
	    n_yvel++;
	    break;
	case DIR_EAST:
	    n_xvel++;
	    break;
	case DIR_WEST:
	    n_xvel--;
	    break;
	case DIR_NORTHEAST:
	    n_yvel--;
	    n_xvel++;
	    break;
	case DIR_NORTHWEST:
	    n_yvel--;
	    n_xvel--;
	    break;
	case DIR_SOUTHEAST:
	    n_yvel++;
	    n_xvel++;
	    break;
	case DIR_SOUTHWEST:
	    n_yvel++;
	    n_xvel--;
	    break;
	default:
	    break;
    }	

    *xvel = n_xvel;
    *yvel = n_yvel;
}

int
OnScreen(int x, int y)
{
    return TRUE;
}

Uint32
TimeLeft(Uint32 next_time)
{
    Uint32 now;

    now = SDL_GetTicks();
    if (next_time < now) {
	return 0;
    } else {
	return next_time - now;
    }
}
