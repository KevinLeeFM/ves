#include "SDL.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define SCREEN_SCALE_RATIO 3 // TODO: implement screen_scale_ratio. Not gonna do it now to minimize potential for bugs

typedef struct Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

typedef struct Screen {
    Color colors[16];

    // the monster expression is equivalent to ceildivide by 2
    // each pixel takes up a nibble
    Uint8 pixels[1 + (((SCREEN_WIDTH * SCREEN_HEIGHT) - 1) / 2)]; 
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    struct timeval tv_draw;
} Screen;

Screen* screen_init();

void screen_free(Screen* screen);

int screen_fill_scanline(Screen* screen, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, Uint8 color);

int screen_pset(Screen* screen, unsigned int x, unsigned int y, Uint8 color);

int screen_rectfill(Screen* screen, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, Uint8 color);

int screen_line(Screen* screen, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, Uint8 color);

void screen_blit(Screen* screen);

int lib_screen_pset(lua_State *L);

int lib_screen_rectfill(lua_State *L);

int lib_screen_line(lua_State *L);

int lib_screen_cset(lua_State *L);