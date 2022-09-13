# VES - Virtual Entertainment System
A WIP toy emulator that runs Lua-based visual applications. Inspired by [PICO-8](https://www.lexaloffle.com/pico-8.php).

Features
- Draw with up to 16 definable colors, changable with `cset`
- `pset`: draw a single pixel
- `rectfill`: draw a filled rectangle
- `line`: draw a line

Meaningful Lua errors will be thrown for improper arguments

## Example
![a screenshot of a sample Lua file running in VES](https://user-images.githubusercontent.com/54872415/189797382-dde46ad5-41c7-46f2-8549-5b8ab77753e2.png)

This is produced by the following Lua program
```lua
-- Start
-- Script: script.lua

Screen = NibbleScreen

-- pico 8 palette
Screen.cset( 0, 0x00, 0x00, 0x00)
Screen.cset( 1, 0x1d, 0x2b, 0x53)
Screen.cset( 2, 0x7e, 0x25, 0x53)
Screen.cset( 3, 0x00, 0x87, 0x51)
Screen.cset( 4, 0xab, 0x52, 0x36)
Screen.cset( 5, 0x5f, 0x57, 0x4f)
Screen.cset( 6, 0xc2, 0xc3, 0xc7)
Screen.cset( 7, 0xff, 0xf1, 0xe8)
Screen.cset( 8, 0xff, 0x00, 0x4d)
Screen.cset( 9, 0xff, 0xa3, 0x00)
Screen.cset(10, 0xff, 0xec, 0x27)
Screen.cset(11, 0x00, 0xe4, 0x36)
Screen.cset(12, 0x29, 0xad, 0xff)
Screen.cset(13, 0x83, 0x76, 0x9c)
Screen.cset(14, 0xff, 0x77, 0xa8)
Screen.cset(15, 0xff, 0xcc, 0xaa)

x1 = 5
y1 = 6
x2 = 59
y2 = 120
c = 0

function _screen_draw(delta)
    Screen.line(x1, y1, x2, y2, c)
    x1 = ((x1 + 2)) % 128
    y1 = ((y1 + 2)) % 128
    x2 = ((x2 + 3)) % 128
    y2 = ((y2 + 3)) % 128
    c = (c + 3) % 16

    Screen.rectfill(x1 // 1.5, y1 // 2, x2 // 2, y2 // 1.5, c)
end
-- End
```

## Roadmap
- Modularize Lua draw functions into namespaces
- Remove most Lua functions that came with the interpreter (similar to PICO-8, to ensure API simplicity)
- Implement custom math functions such as ceil, clamp, sin (period of 2*pi), sin1 (period of 1)
- Spritesheet support (exact implementation detail to be determined)
- Implement support for bitmap font (exact implementation detail to be determined). Font spritesheet will likely be stored as bitmap similar to [Unifont](http://unifoundry.com/unifont/). The more language support, the better.

Still unsure
- Add language support for a dialect of Forth
- Customizable constraints, similar to [Pixel Vision 8](https://pixelvision8.github.io/Website/)
