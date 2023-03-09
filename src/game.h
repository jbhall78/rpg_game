#ifndef _GAME_H
#define _GAME_H

/*
 * Defines
 */
#define TRUE 1
#define FALSE 0

#define TICK_INTERVAL 30 

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
//#define SCREEN_WIDTH 800
//#define SCREEN_HEIGHT 600

#define TILE_WIDTH  32
#define TILE_HEIGHT 32

#define MAP_WIDTH 30
#define MAP_HEIGHT 30

#define PASS_TILE "gfx/tiles/pass.bmp"
#define NOPASS_TILE "gfx/tiles/nopass.bmp"
#define LAYER_TILE "gfx/tiles/layer.bmp"

#define PLAYER "gfx/player/player.bmp"
#define PLAYER_FRAMES 3
#define MOVEMENT_FRAMES 6

#define NPC1 "gfx/npc/npc1.bmp"
#define NPC2 "gfx/npc/npc2.bmp"
#define NPC_FRAMES 3
#define NPC_MOVEMENT_FRAMES 18
#define NUM_NPCS 10 // number of NPCs to create

#define SCRIPT_BATTLE 1
#define SCRIPT_NPC_MSG 2
#define SCRIPT_PLAYER_MSG 3
#define SCRIPT_PLAYER_MENU 4

#define FONT "gfx/font/font.bmp"

#define VEL_UP -1
#define VEL_DOWN 1
#define VEL_LEFT -1
#define VEL_RIGHT 1
#define VEL_STOP 0

#define NUM_DIRECTIONS 8

#define DIR_NONE      0
#define DIR_SOUTH     0
#define DIR_NORTH     1
#define DIR_WEST      2
#define DIR_EAST      3
#define DIR_NORTHEAST 4
#define DIR_NORTHWEST 5
#define DIR_SOUTHEAST 6
#define DIR_SOUTHWEST 7


/*
 * Keyboard input modes... also changes how the main game loop runs a little
 */
#define MODE_PLAY 1
#define MODE_GAMEMENU 2
#define MODE_NPC_INTERACTION 3
#define MODE_BATTLE 4

#define WIN_BORDER_THICKNESS 7

#define MENU_X_OFFSET 30
#define MENU_Y_OFFSET 20

#define WIN_X_OFFSET 10
#define WIN_Y_OFFSET 10

#define LINE_SPACING 5

/*
 * Our structures / Other type definitions
 */
typedef struct {
    SDL_Surface *surface;

    int x, y;
    int layer; // current layer we are sitting on
    float xmod, ymod;

    int xvel, yvel;
    int g_xvel; // these are different then above, these are 'good' values the way we are
    int g_yvel; // ACTUALLY going to move vs. the way we WANT to move 
                // ps: i really hate doing it like this, but it works.
    int dir; // player direction -- used for rendering frames

    int frames; // frames left to play
    int change_time;
    int cur_frame;

    // player data
    int health, maxhealth;
    int level, experience, next_level;
    int strength, dexterity;
} G_Player;

typedef struct {
    SDL_Surface *trans; // the transparent surface for our window
    SDL_Surface *border; // the border surface for our window

    // window position / size data
    int x, y;
    int w, h;

    char *title;
} G_Window;

typedef struct {
    SDL_Surface *surface;
    int w, h;

    // our font doesnt cover every ascii character, so this is the offset from the beginning
    // of the ascii chart in which our font starts
    int offset; 
} G_Font;

typedef struct {
    G_Window *win;
    G_Font *font;
    char *text;
} G_TextBox;

typedef int G_Function();

typedef struct {
    SDLKey sym; // item hotkey
    char *name;
    G_Function *function;
} G_MenuItem;

typedef struct {
    G_Window *win;         // window we draw to
    G_MenuItem *items;     // pointer to array of menu items
    G_Font *font;          // font for this menu
    SDL_Surface *sel_surf; // surface for our selection thing

    int len;
    int sel; // currently selected item
} G_Menu;

typedef struct _G_Script G_Script;

struct _G_Script {
    int type;
    
    char *desc; // description for a menu item
    char *msg; // msg for npc/player msgs

    G_TextBox *box;
    G_Menu *menu;
    
    int n_options;
    G_Script **options;
};

typedef struct {
    SDL_Surface *surface;

    G_Script *contact_script;
    G_Script *use_script;

    int x, y;
    float xmod, ymod;
    int layer;

    int xvel, yvel;
    int dir;

    int change_time;
    int frames;
    int cur_frame;
} G_NPC;

typedef struct {
    SDL_Surface **surfaces;
    int n_surfaces;

    int passable;
    int occupied;
} G_Tile;

typedef struct {
    G_Tile **tiles;
    SDL_Surface *surface;
    G_NPC *npcs;

    int w, h;
    int view_x, view_y; // viewport

    int xvel, yvel;     // which direction the map is moving while scrolling
    float xmod, ymod;   // offsets for draws while scrolling

    int frames; // frames left to shift of map
} G_Map;

typedef struct {
    int frames; // number of frames played on this screen
    Uint32 flags;
    int bpp;
    int w, h;
    int tiles_x, tiles_y;

    SDL_Surface *surface;

    G_Font *font;
} G_Screen;

typedef struct {
    int shutdown;
    int mode;

    // lame keyboard hack stuff
    int up_down;
    int down_down;
    int left_down;
    int right_down;

    G_Menu *game_menu;
    G_Map *map;
    G_Player *player;
    G_Screen *screen;

    // current npc we are interacting with
    int npc_id;
    // current script we are running
    G_Script *script;

} G_Game;

/************************
 * Function Definitions *
 ************************/

/* main.c */
// core
G_Game *CreateGame(int argc, char **argv);
void UpdateGame(G_Game *game);
void DrawGame(G_Game *game);

int GetGameMode(G_Game *game);
void SetGameMode(G_Game *game, int mode);

void ScheduleShutdown(G_Game *game);
int ShutdownScheduled(G_Game *game);

void ProcessEvents(G_Game *game);

// misc
int GetDirectionFromVelocities(int xvel, int yvel);
void GetVelocitiesFromDirection(int dir, int *xvel, int *yvel);
int OnScreen(int x, int y);

// timing
Uint32 TimeLeft(Uint32 next_time);

/* map.c */
G_Map *CreateMap(G_Screen *screen);
void UpdateMap(G_Screen *screen, G_Player *player, G_Map *map);
void DrawMap(G_Screen *screen, G_Player *player, G_Map *map);

// map utility 
int CanMoveTo(G_Map *map, int x, int y);
int GetRandomDirection(G_Map *map, int x, int y);
void GetRandomMapPosition(G_Map *map, int *px, int *py);
int IsOccupied(G_Map *map, int x, int y);

/* npc.c */
G_NPC *CreateNPCs(G_Map *map);
void UpdateNPCs(G_Map *map);
void DrawNPCs(SDL_Surface *surface, G_Map *map, int layer);

// npc script stuff
G_Script *CreateScript(int type, char *desc, char *msg, G_Script **options, int n_options);
void DestroyScript(G_Script *script);
void UpdateScript(void);
void DrawScript(G_Game *game);
void StepScript(G_Game *game, int option);

int InitiateNPCContact(G_Game *game, int npc_id);
int InitiateNPCUse(G_Game *game, int npc_id);

/* player.c */
G_Player *CreatePlayer(G_Screen *screen, G_Map *map);
void UpdatePlayer(G_Game *game);
void DrawPlayer(SDL_Surface *surface, G_Map *map, G_Player *player);
void PlayerUse(G_Game *game);

/* screen.c */
G_Screen *CreateScreen(int w, int h, int bpp, Uint32 flags);

void UpdateFPS(G_Screen *screen);
void DrawFPS(G_Screen *screen);

void DrawPixel(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B);
Uint32 GetPixel(SDL_Surface *surface, int x, int y);

/* util.c */
void *Malloc(size_t size);
void *Calloc(size_t elements, size_t size);
void Free(void *ptr);

/* window.c */
G_Font *CreateFont(char *filename);
void DrawText(G_Screen *screen, int x, int y, char *str);

// window
G_Window *CreateWindow(G_Screen *screen, char *title, int x, int y, int w, int h);
void DrawWindow(G_Screen *screen, G_Window *win);
void DestroyWindow(G_Window *win);

// menu
G_Menu *CreateMenu(G_Screen *screen, G_MenuItem *items, char *title, int x, int y);
void UpdateMenu(G_Menu *menu);
void DrawMenu(G_Screen *screen, G_Menu *menu);
void DestroyMenu(G_Menu *menu);

// textbox
G_TextBox *CreateTextBox(G_Screen *screen, char *title, char *text, int x, int y);
void DrawTextBox(G_Screen *screen, G_TextBox *box);
void DestroyTextBox(G_TextBox *box);

// menu callbacks
int GameMenuQuitGame(G_Game *game);
int GameMenuReturnToGame(G_Game *game);

#endif /* _GAME_H */
