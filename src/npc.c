#include "includes.h"
#include "game.h"

G_NPC *
CreateNPCs(G_Map *map)
{
    G_NPC *npcs;
    SDL_Surface *npc1;
    SDL_Surface *npc2;
    int i;
    G_Script *nodes[10];
    G_Script *m_nodes[10];
    G_Script *contact;
    G_Script *use;

    npcs = Calloc(NUM_NPCS, sizeof(G_NPC));

    if ((npc1 = SDL_LoadBMP(NPC1)) == NULL) {
	fprintf(stderr, "Can't load NPC image!\n");
	return NULL;
    }
    if ((npc2 = SDL_LoadBMP(NPC2)) == NULL) {
	fprintf(stderr, "Can't load NPC image!\n");
	return NULL;
    }

    // make the contact script short cuz it might just be run cuz we accidently run into someone
    contact = CreateScript(SCRIPT_NPC_MSG, NULL, "Don't touch me!", NULL, 0);

    m_nodes[0] = CreateScript(SCRIPT_BATTLE,"Kick NPC Ass", "You decided to kick npc ass!", NULL,0);
    m_nodes[1] = CreateScript(SCRIPT_PLAYER_MSG,"Run Away", "You run away like a wussy!", NULL,0);

    nodes[3] = CreateScript(SCRIPT_PLAYER_MENU, NULL, "Choose What You Want To Do", m_nodes, 2);
    nodes[2] = CreateScript(SCRIPT_NPC_MSG, NULL, "Yes I Will!", &nodes[3], 1);
    nodes[1] = CreateScript(SCRIPT_PLAYER_MSG, NULL, "No You Wont!", &nodes[2], 1);
    nodes[0] = CreateScript(SCRIPT_NPC_MSG, NULL, "I Will Defeat You!", &nodes[1], 1);

    use = nodes[0];

    for (i = 0; i < NUM_NPCS; i++) {
	GetRandomMapPosition(map, &npcs[i].x, &npcs[i].y);

	map->tiles[npcs[i].y][npcs[i].x].occupied = TRUE;

	npcs[i].xvel = VEL_STOP;
	npcs[i].yvel = VEL_STOP;
	npcs[i].layer = 0;

	npcs[i].dir = DIR_NONE;

	if ((rand() % 4) == 1) {
	    npcs[i].contact_script = contact;
	    npcs[i].surface = npc1;
	} else {
	    npcs[i].use_script = use;
	    npcs[i].surface = npc2;
	}

	SDL_SetColorKey(npcs[i].surface, SDL_SRCCOLORKEY, GetPixel(npcs[i].surface, 0, 0));

    }

    return npcs;
}

void
UpdateNPCs(G_Map *map)
{
    int i, dir;
    float x_step = (float)TILE_WIDTH / (float)NPC_MOVEMENT_FRAMES;
    float y_step = (float)TILE_HEIGHT / (float)NPC_MOVEMENT_FRAMES;
    int frame_frames = NPC_MOVEMENT_FRAMES / NPC_FRAMES; // number of frames a particular frame
                                                        // appears on the screen
    G_NPC *npcs = map->npcs;

    for (i = 0; i < NUM_NPCS; i++) {
	npcs[i].xmod = 0;
	npcs[i].ymod = 0;

	// if this npc is already moving .. move on
	if (npcs[i].frames) {
    	    if (npcs[i].xvel) {
		if (npcs[i].xvel > 0) {
	    	    npcs[i].xmod = (-1*TILE_WIDTH) + ((NPC_MOVEMENT_FRAMES-npcs[i].frames)*x_step);
		} else if (npcs[i].xvel < 0) {
		    npcs[i].xmod = (1*TILE_WIDTH) - ((NPC_MOVEMENT_FRAMES-npcs[i].frames)*x_step);
		}
	    }
	    if (npcs[i].yvel) {
		if (npcs[i].yvel > 0) {
		    npcs[i].ymod = (-1*TILE_HEIGHT) + ((NPC_MOVEMENT_FRAMES-npcs[i].frames)*y_step);
		} else if (npcs[i].yvel < 0) {
		    npcs[i].ymod = (1*TILE_HEIGHT) - ((NPC_MOVEMENT_FRAMES-npcs[i].frames)*y_step);
		}
	    }

	    if (! npcs[i].change_time) {
		if (++npcs[i].cur_frame >= NPC_FRAMES) {
		    npcs[i].cur_frame = 0;
		}
		npcs[i].change_time = frame_frames;
	    } else {
		npcs[i].change_time--;
	    }

	    npcs[i].frames--;
	    continue;
	} 
     	npcs[i].cur_frame = 0;
	npcs[i].change_time = 0;

	// we are not moving. lets see if we should move
	// we have a 1 in 100 chance of moving
	if ((rand() % 100) == 1) {
	    if ((dir = GetRandomDirection(map, npcs[i].x, npcs[i].y)) < 0) {
		continue;
	    }

	    // update our direction
	    GetVelocitiesFromDirection(dir, &npcs[i].xvel, &npcs[i].yvel);
	    if ((dir == DIR_NORTHWEST) || (dir == DIR_NORTHEAST))
		dir = DIR_NORTH;
	    if ((dir == DIR_SOUTHEAST) || (dir == DIR_SOUTHWEST))
		dir = DIR_SOUTH;

	    npcs[i].dir = dir;

	    map->tiles[npcs[i].y][npcs[i].x].occupied = FALSE;
	    npcs[i].x += npcs[i].xvel;
	    npcs[i].y += npcs[i].yvel;
	    map->tiles[npcs[i].y][npcs[i].x].occupied = TRUE;

	    npcs[i].xmod = -1 * (npcs[i].xvel * TILE_WIDTH);
	    npcs[i].ymod = -1 * (npcs[i].yvel * TILE_HEIGHT);
	    
	    npcs[i].frames = NPC_MOVEMENT_FRAMES;
	}
    }
}

void
DrawNPCs(SDL_Surface *surface, G_Map *map, int layer)
{
    SDL_Rect dst_area;
    SDL_Rect src_area;
    int i;
    G_NPC *npcs = map->npcs;

    for (i = 0; i < NUM_NPCS; i++) {
	if ((npcs[i].layer == layer) && OnScreen(npcs[i].x, npcs[i].y)) {
	    src_area.w = TILE_WIDTH;
	    src_area.h = TILE_HEIGHT;
	    src_area.x = npcs[i].cur_frame * TILE_WIDTH;
	    src_area.y = npcs[i].dir * TILE_HEIGHT;
	    
	    dst_area.w = TILE_WIDTH;
	    dst_area.h = TILE_HEIGHT;
	    dst_area.x = ((npcs[i].x - (map->view_x - 1)) * TILE_WIDTH) + npcs[i].xmod;
	    dst_area.y = ((npcs[i].y - (map->view_y - 1)) * TILE_HEIGHT) + npcs[i].ymod;

	    SDL_BlitSurface(npcs[i].surface, &src_area, surface, &dst_area);
	} 
    }
}

G_Script *
CreateScript(int type, char *desc, char *msg, G_Script **options, int n_options)
{
    G_Script *script;
    int i;
    
    script = Malloc(sizeof(G_Script));
    
    script->type = type;
    if (desc)
    	script->desc = strdup(desc);
    if (msg)
    	script->msg = strdup(msg);
    
    script->n_options = n_options;
    script->options = Calloc(n_options, sizeof(G_Script *));
    for (i = 0; i < n_options; i++) {
    	script->options[i] = options[i];
    }
    
    return script;
}

void
DestroyScript(G_Script *script)
{
    int i;
    
    Free(script->desc);
    Free(script->msg);
    
    if (script->options) {
    	for (i = 0; i < script->n_options; i++) {
    	    DestroyScript(script->options[i]);
    	}
    	Free(script->options);
    }
    
    Free(script);
}

void
UpdateScript(void)
{

}

void
DrawScript(G_Game *game)
{
    G_Screen *screen = game->screen;
    G_Script *script = game->script;
    G_MenuItem *items;
/*    G_Script *next_node = NULL;
    int i;
    char *buf;
    int option;*/

    switch (script->type) {
        case SCRIPT_BATTLE:
/*            printf("Battle: player vs. npc! - %s\n", script->msg);
            PromptForInput("--- Press Return To Continue ---");
            if (script->options)
                next_node = script->options[0];*/
            break;

        case SCRIPT_NPC_MSG:
	    if (script->box == NULL) {
		script->box = CreateTextBox(screen, NULL, script->msg, 50, 50);
	    } 
	    DrawTextBox(screen, script->box);

            break;
        case SCRIPT_PLAYER_MSG:
	    if (script->box == NULL) {
		script->box = CreateTextBox(screen, NULL, script->msg, 50, 300);
	    } 
	    DrawTextBox(screen, script->box);

            break;

        case SCRIPT_PLAYER_MENU:
	    if (script->menu == NULL) {
		int i;

		items = Calloc(script->n_options + 1, sizeof(G_MenuItem));

		for (i = 0; i < script->n_options; i++) {
		    items[i].sym = SDLK_UNKNOWN;
		    items[i].name = script->options[i]->desc;
		    items[i].function = NULL;
		}
		
		script->menu = CreateMenu(screen, items, script->msg, 50, 300);
	    }
	    DrawMenu(screen, script->menu);

            break;

        default:
            break;
    }

//    return next_node;
}

void
StepScript(G_Game *game, int option)
{
    G_Script *script = game->script;
    G_Script *next_node = NULL;

    switch (script->type) {
        case SCRIPT_NPC_MSG:
	    if (script->box) {
		DestroyTextBox(script->box);
		script->box = NULL;
	    }

	    next_node = script->options[option];

	    break;
	case SCRIPT_PLAYER_MSG:
	    if (script->box) {
		DestroyTextBox(script->box);
		script->box = NULL;
	    }

	    next_node = script->options[option];
	    
	    break;
	case SCRIPT_PLAYER_MENU:
	    if (script->menu) {
		DestroyMenu(script->menu);
		script->menu = NULL; // XXX - WHY THE FUCK IS THIS GOING ON?
	    }

	    next_node = script->options[option];

	    break;
	case SCRIPT_BATTLE:
	    printf("battle!\n");

	    break;
	default:
	    break;
    }

    if (next_node == NULL) {
	game->script = NULL;
	game->npc_id = -1;

    	SetGameMode(game, MODE_PLAY); 
    } else {
	game->script = next_node;
    }
}

int
InitiateNPCContact(G_Game *game, int npc_id)
{
    G_NPC *npcs = game->map->npcs;
    G_Player *player = game->player;

    if (npcs[npc_id].contact_script == NULL)
	return FALSE;

    SetGameMode(game, MODE_NPC_INTERACTION); 

    game->up_down = FALSE;
    game->down_down = FALSE;
    game->left_down = FALSE;
    game->right_down = FALSE;

    player->dir = GetDirectionFromVelocities(player->xvel, player->yvel);
    player->xvel = player->yvel = 0;
    player->g_xvel = player->g_yvel = 0;
    player->xmod = player->ymod = 0;
    player->frames = 0;
    player->cur_frame = 0;

    game->npc_id = npc_id;
    game->script = npcs[npc_id].contact_script;

    return TRUE;
}

int
InitiateNPCUse(G_Game *game, int npc_id)
{
    G_NPC *npcs = game->map->npcs;
    G_Player *player = game->player;

    if (npcs[npc_id].use_script == NULL)
	return FALSE;

    SetGameMode(game, MODE_NPC_INTERACTION); 

    game->npc_id = npc_id;
    game->script = npcs[npc_id].use_script;

    game->up_down = FALSE;
    game->down_down = FALSE;
    game->left_down = FALSE;
    game->right_down = FALSE;

    player->xvel = player->yvel = 0;
    player->g_xvel = player->g_yvel = 0;
    player->xmod = player->ymod = 0;
    player->frames = 0;
    player->cur_frame = 0;

    return TRUE;
}
