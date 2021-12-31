/* Helpful article: https://lucasklassmann.com/blog/2019-02-02-how-to-embeddeding-lua-in-c/*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <sys/time.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "SDL.h"

#include "nblscreen.h"

// equivalent to ceil(x / y)
int ceildivide(int x, int y) {
    return 1 + ((x - 1) / y);
}

int main(int argc, char** argv) {
    printf("This is VES Emulator\n");

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char* filename = argv[1];

    ves_screen = screen_init();
	
    lua_State* L = luaL_newstate();

    luaL_openlibs(L);

    // Screen library
    lua_newtable(L);
    luaL_setfuncs(L, ScreenLib, 0);
    lua_setglobal(L, "NibbleScreen");

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

    // main application loop
    SDL_Event event;
    struct timeval tv_draw_current;
    unsigned int delta_draw;
    memset(&tv_draw_current, 0, sizeof(struct timeval));

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        gettimeofday(&tv_draw_current, NULL);
        delta_draw = (tv_draw_current.tv_sec - ves_screen->tv_draw.tv_sec) * 1000 + (tv_draw_current.tv_usec - ves_screen->tv_draw.tv_usec) / 1000;
        ves_screen->tv_draw = tv_draw_current;

        // evaluate draw function: _screen_draw(delta)
        lua_getglobal(L, "_screen_draw");
        if (lua_isfunction(L, -1)) {
            lua_pushinteger(L, delta_draw);
            if (lua_pcall(L, 1, 0, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            } else {
                printf("! Lua error: %s\n", lua_tostring(L, lua_gettop(L)));
                break;
            }
        }

        // TODO: remove clear later since entire screen is redrawn anyways
        SDL_SetRenderDrawColor(ves_screen->renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(ves_screen->renderer);
        screen_blit(ves_screen);
        SDL_RenderPresent(ves_screen->renderer);
    }

    screen_free(ves_screen);

    lua_close(L);
    atexit(SDL_Quit); // it is not wise to call this from a library or other dynamically loaded code

	return 0;
}