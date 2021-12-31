#include "nblscreen.h"

#include <assert.h>

#include "SDL.h"

Screen* ves_screen;

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
    screen->colors[1].r = 0xFF;
    screen->colors[1].g = 0x00;
    screen->colors[1].b = 0x00;
    screen->colors[2].r = 0x00;
    screen->colors[2].g = 0xFF;
    screen->colors[2].b = 0x00;
    screen->colors[3].r = 0x00;
    screen->colors[3].g = 0x00;
    screen->colors[3].b = 0xFF;

    return screen;
}


void screen_free(Screen* screen) {
    if (screen != NULL) {
        SDL_DestroyRenderer(screen->renderer);
        SDL_DestroyWindow(screen->window);
        free(screen);
    }
}

// Return 0 on success, 1 on failure.
// The pixel (x2, y2) is inclusive.
int screen_fill_scanline(Screen* screen, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, Uint8 color) {
    // TODO: surround this assert in debug
    assert(color < 16 && x1 < SCREEN_WIDTH && x2 < SCREEN_WIDTH && y1 < SCREEN_HEIGHT && y2 < SCREEN_HEIGHT);

    int coord_start = (x1 + y1 * SCREEN_WIDTH);
    int coord_end = (x2 + y2 * SCREEN_WIDTH);

    if (coord_start > coord_end) {
        int temp = coord_start;
        coord_start = coord_end;
        coord_end = temp;
    }

    // printf("AAAA: %u, %u\n", coord_start, coord_end);

    if (coord_start % 2) { // if odd
        screen->pixels[coord_start / 2] = (screen->pixels[coord_start / 2] & 0x0F) | (color << 4);
        coord_start += 1;
    }

    if (!(coord_end % 2)) { // if even
        screen->pixels[coord_end / 2] = (screen->pixels[coord_end / 2] & 0xF0) | (color);
        coord_end -= 1;
    }

    if (coord_start <= coord_end) {
        memset(screen->pixels + (coord_start / 2), color | (color << 4), ((coord_end - coord_start) / 2) + 1);
    }

    // printf("(pixel at %u, %u): %x\n", x1, y1, screen->pixels[(x1 + y1 * SCREEN_WIDTH)/ 2]);
    // printf("(pixel at %u, %u): %x\n", x2, y2, screen->pixels[(x2 + y2 * SCREEN_WIDTH)/ 2]);

    return 0;
}

int screen_pset(Screen* screen, unsigned int x, unsigned int y, Uint8 color) {
    // TODO: surround this assert in debug
    assert(color < 16 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT);
    const int coord = (x + y * SCREEN_WIDTH);
    const Uint8 pixel = screen->pixels[coord / 2];
    screen->pixels[coord / 2] = coord % 2 ? ((pixel & 0x0F) | (color << 4)) : ((pixel & 0xF0) | (color));

    return 0;
}

// (x2, y2) is inclusive
int screen_rectfill(Screen* screen, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, Uint8 color) {
    assert(color < 16 && x1 < SCREEN_WIDTH && x2 < SCREEN_WIDTH && y1 < SCREEN_HEIGHT && y2 < SCREEN_HEIGHT);

    for (int y = y1; y <= y2; y++) {
        screen_fill_scanline(screen, x1, y, x2, y, color);
    }

    return 0;
}

int screen_line(Screen* screen, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, Uint8 color) {
    assert(color < 16 && x1 < SCREEN_WIDTH && x2 < SCREEN_WIDTH && y1 < SCREEN_HEIGHT && y2 < SCREEN_HEIGHT);
    
    // Bresenham's line algorithm because I'm too lazy to reinvent the wheel
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;

    int err = dx + dy;

    // printf("x1, y1, x2, y2: %u, %u, %u, %u\n", x1, y1, x2, y2);

    while (1) {
        // printf("x1, y1: %u, %u\n", x1, y1);
        if (x1 > x2 || y1 > y2) {
            screen_pset(screen, x2, y2, color);
            break;
        }
        screen_pset(screen, x1, y1, color);
        if (x1 == x2 && y1 == y2) { break; }

        if (2 * err >= dy) {
            err += dy;
            x1 += sx;
        }

        if (2 * err <= dx) {
            err += dx;
            y1 += sy;
        }
    }

    return 1;
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

int lib_screen_pset(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int c = luaL_checkinteger(L, 3);

    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return luaL_error(L, "Screen error: pset coordinate (x,y) out of bound");
    } else if (c < 0 || c >= 16) {
        return luaL_error(L, "Screen error: pset color index c out of bound");
    }

    screen_pset(ves_screen, x, y, c);
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

    screen_rectfill(ves_screen, x1, y1, x2, y2, c);
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

    screen_line(ves_screen, x1, y1, x2, y2, c);
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

    ves_screen->colors[c].r = r;
    ves_screen->colors[c].g = g;
    ves_screen->colors[c].b = b;

    // printf("%u, %u, %u\n", screen->colors[c].r , screen->colors[c].g , screen->colors[c].b );

    return 1;
}

const luaL_Reg ScreenLib[] = {
    {"pset", lib_screen_pset},
    {"rectfill", lib_screen_rectfill},
    {"line", lib_screen_line},
    {"cset", lib_screen_cset},
    {NULL, NULL}
};