/* Helpful article: https://lucasklassmann.com/blog/2019-02-02-how-to-embeddeding-lua-in-c/*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <sys/time.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "nblscreen.h"

#include "SDL.h"

// equivalent to ceil(x / y)
int ceildivide(int x, int y) {
    return 1 + ((x - 1) / y);
}

#define NBLSCREEN_PTR_FIELD "_ptr"
#define SCREEN_NUM 2

Screen* global_screens[SCREEN_NUM];

int lib_screen_pset(lua_State *L) {
    if (lua_gettop(L) != 4) {
        return luaL_error(L, "Screen error: pset expects 4 arguments (3 for : operator)");
    }
    luaL_checktype(L, 1, LUA_TTABLE);
    if (lua_getfield(L, -4, NBLSCREEN_PTR_FIELD) != LUA_INT_TYPE) {
        return luaL_error(L, "Screen error: the table contained wrong types. This is likely due to modification of values that should not be changed.");
    }

    Screen* screen = global_screens[lua_getfield(L, -4, NBLSCREEN_PTR_FIELD)]

    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int c = luaL_checkinteger(L, 4);

    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return luaL_error(L, "Screen error: pset coordinate (x,y) out of bound");
    } else if (c < 0 || c >= 16) {
        return luaL_error(L, "Screen error: pset color index c out of bound");
    }

    screen_pset(screen, x, y, c);
    return 1;
}

int lib_screen_rectfill(lua_State *L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    int c = luaL_checkinteger(L, 5);

    if (x1 < 0 || x1 >= SCREEN_WIDTH || y1 < 0 || y1 >= SCREEN_HEIGHT) {
        return luaL_error(L, "Screen error: rectfill coordinate (x1,y1) out of bound");
    } else if (x2 < 0 || x2 >= SCREEN_WIDTH || y2 < 0 || y2 >= SCREEN_HEIGHT) {
        return luaL_error(L, "Screen error: rectfill coordinate (x2,y2) out of bound");
    } else if (c < 0 || c >= 16) {
        return luaL_error(L, "Screen error: rectfill color index c out of bound");
    }

    screen_rectfill(screen, x1, y1, x2, y2, c);
    return 1;
}

int lib_screen_line(lua_State *L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    int c = luaL_checkinteger(L, 5);

    if (x1 < 0 || x1 >= SCREEN_WIDTH || y1 < 0 || y1 >= SCREEN_HEIGHT) {
        return luaL_error(L, "Screen error: line coordinate (x1,y1) out of bound");
    } else if (x2 < 0 || x2 >= SCREEN_WIDTH || y2 < 0 || y2 >= SCREEN_HEIGHT) {
        return luaL_error(L, "Screen error: line coordinate (x2,y2) out of bound");
    } else if (c < 0 || c >= 16) {
        return luaL_error(L, "Screen error: line color index c out of bound");
    }

    screen_line(screen, x1, y1, x2, y2, c);
    return 1;
}

int lib_screen_cset(lua_State *L) {
    int c = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);

    if (r < 0 || r >= 256 || g < 0 || g >= 256 || b < 0 || b >= 256) {
        return luaL_error(L, "Screen error: cset color (r, g, b) out of bound");
    }

    if (c < 0 || c >= 16) {
        return luaL_error(L, "Screen error: cset color index c out of bound");
    }

    screen->colors[c].r = r;
    screen->colors[c].g = g;
    screen->colors[c].b = b;

    // printf("%u, %u, %u\n", screen->colors[c].r , screen->colors[c].g , screen->colors[c].b );

    return 1;
}

static int iohint(lua_State *L) {
    char* ioname = luaL_checkstring(L, 1);
    if (strcmp(ioname, "NibbleScreen") == 0) {
        return iohint_screen(L);
    }

    return luaL_error(L, "VES IO error: The IO \"%s\" cannot be found", ioname);
}

// 1 means success, others are Lua error
int iohint_screen(lua_State *L) {
    if (global_screen != NULL) {
        return luaL_error(L, "VES IO error: VES does not satisfy the device requirement. NibbleScreen can only be initialized once.");
    }

    int n = lua_gettop(L);    /* number of arguments */
    if (lua_gettop != 3) {
        return luaL_error(L, "NibbleScreen error: iohint of NibbleScreen requires the following arguments: (\"NibbleScreen\", <width>, <height>).");
    }

    global_screen = screen_init();
    lua_createtable(L, 0, 5);
    lua_pushinteger(L, global_screen);
    lua_setfield(L, -2, NBLSCREEN_PTR_FIELD); /* setfield pops from the stack */
    lua_pushcfunction(L, lib_screen_pset);
    lua_setfield(L, -2, "pset");
    lua_pushcfunction(L, lib_screen_rectfill);
    lua_setfield(L, -2, "rectfill");
    lua_pushcfunction(L, lib_screen_line);
    lua_setfield(L, -2, "line");
    lua_pushcfunction(L, lib_screen_cset);
    lua_setfield(L, -2, "cset");

    return 1;
}

const luaL_Reg ScreenLib[] = {
    {"pset", lib_screen_pset},
    {"rectfill", lib_screen_rectfill},
    {"line", lib_screen_line},
    {"cset", lib_screen_cset},
    {NULL, NULL}
};

int main(int argc, char** argv) {
    printf("This is VES Emulator\n");

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char* filename = argv[1];

    screen = screen_init();
	
    lua_State* L = luaL_newstate();

    luaL_openlibs(L);

    // Screen library
    // TODO: remove later
    lua_newtable(L);
    luaL_setfuncs(L, ScreenLib, 0);
    lua_setglobal(L, "ScreenOutdated");

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
        delta_draw = (tv_draw_current.tv_sec - screen->tv_draw.tv_sec) * 1000 + (tv_draw_current.tv_usec - screen->tv_draw.tv_usec) / 1000;
        screen->tv_draw = tv_draw_current;

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