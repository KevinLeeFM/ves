/* Helpful article: https://lucasklassmann.com/blog/2019-02-02-how-to-embeddeding-lua-in-c/*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "SDL.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define SCREEN_SCALE_RATIO 3 // TODO: implement screen_scale_ratio. Not gonna do it now to minimize potential for bugs

// equivalent to ceil(x / y)
int ceildivide(int x, int y) {
    return 1 + ((x - 1) / y);
}

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
} Screen;

Screen* screen_init() {
    // initialize SDL video if it isn't already initialized
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
            exit(3);
        }
    }

    Screen* screen = malloc(sizeof(Screen));
    memset(screen, 0, sizeof(Screen));

    // see here for window flags: https://wiki.libsdl.org/SDL_CreateWindow
    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &(screen->window), &(screen->renderer))) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        exit(3);
    }

    // TODO: remove later, set zeroth index as black and first index as red
    screen->colors[0].r = 0x00;
    screen->colors[0].g = 0x00;
    screen->colors[0].b = 0x00;
    screen->colors[1].r = 0x00;
    screen->colors[1].g = 0xFF;
    screen->colors[1].b = 0x00;

    return screen;
}

void screen_free(Screen* screen) {
    if (screen != NULL) {
        SDL_DestroyRenderer(screen->renderer);
        SDL_DestroyWindow(screen->window);
        free(screen);
    }
}

// return 0 on success, 1 on failure
int screen_fill_scanline(Screen* screen, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
    // TODO: surround this assert in debug
    assert(color < 16 && x1 < SCREEN_WIDTH && x2 < SCREEN_WIDTH && y1 < SCREEN_HEIGHT && y2 < SCREEN_HEIGHT);
    return 0;
}

void screen_blit(Screen* screen) {
    Uint8 byte;
    Uint8 pixel;
    Color color;
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.h = 1;
    rect.w = 1;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            // get the byte that the nibble is in
            byte = (screen->pixels[(x + y * SCREEN_WIDTH) / 2]);
            // read the nibble
            pixel = ((x + y * SCREEN_WIDTH) % 2) ? byte >> 4 : byte & 0x0F;

            /* if (x < 7 && y < 1) {
                printf("(%u, %u): %u\n", x, y, pixel);
            } */

            assert(pixel < 16); // TODO: surround with debug later
            color = screen->colors[pixel];
            SDL_SetRenderDrawColor(screen->renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

            rect.x = x;
            rect.y = y;
            SDL_RenderFillRect(screen->renderer, &rect);
        }
    }
}

int testme(lua_State *L) {
    int a = luaL_checkinteger(L, 1);
    printf("A C function is called from Lua! Argument: %u\n", a);
    return 1;
}

const luaL_Reg ScreenLib[] = {
    { "testme", testme },
    { NULL, NULL }
};

int main(int argc, char** argv) {
    printf("This is ThunderVM\n");

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char* filename = argv[1];

    Screen* screen = screen_init();
	
    lua_State* L = luaL_newstate();

    luaL_openlibs(L);

    // Screen library
    lua_newtable(L);
    luaL_setfuncs(L, ScreenLib, 0);
    lua_setglobal(L, "Screen");

    // Allows only math and string libraries to be used
    // luaopen_io(L);
    // luaopen_math(L);
    // luaopen_string(L);

    // run everything not inside of a function
    if (luaL_dofile(L, filename) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    } else {
        printf("! Lua error: %s\n", lua_tostring(L, lua_gettop(L)));
    }

    screen->pixels[2] = 0x10;

    // main application loop
    SDL_Event event;
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        // evaluate draw function
        lua_getglobal(L, "_screen_draw");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            }
        }

        SDL_SetRenderDrawColor(screen->renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(screen->renderer);
        screen_blit(screen);
        SDL_RenderPresent(screen->renderer);
    }

    screen_free(screen);

    lua_close(L);
    atexit(SDL_Quit); // it is not wise to call this from a library or other dynamically loaded code

	return 0;
}