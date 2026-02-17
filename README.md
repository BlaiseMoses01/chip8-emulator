# CHIP-8 Emulator (SDL2 and C++)

A CHIP-8 emulator written in C++ with SDL2 rendering/input/audio and an in-app debugger overlay.

## Core Features
- 64x32 CHIP-8 display rendering
- 16-key keypad input mapping
- CHIP-8 timers at 60Hz
- Sound timer beep (SDL audio callback)
- Debugger controls:
  - Run/Pause
  - Step one CPU cycle
  - Step one render frame
  - Overlay showing memory state

## Demo 


## Dependencies
- C++20 compiler
- CMake (3.16+)
- SDL2
- SDL2_ttf

On Ubuntu/Debian:
```bash
sudo apt install build-essential cmake libsdl2-dev libsdl2-ttf-dev
```

## Build
From repo root:
```bash
cmake -S src -B build
cmake --build build -j
```

## Run
Usage:
```bash
./build/chip8 <rom_path>
```
example: play pong
```bash
./build/chip8 game-roms/pong.ch8
```

## Controls

### CHIP-8 keypad

  I used the most common key mapping based on the guides I referenced
- `Keyboard` -> `Chip-8 ` 
- `1 2 3 4 ` -> `1 2 3 C`
- `Q W E R ` -> `4 5 6 D`
- `A S D F ` -> `7 8 9 E`
- `Z X C V ` -> `A 0 B F`

### Emulator / debugger
- `Esc` quit (kill VM)
- `F1` toggle debug overlay panel
- `F2` run/pause execution
- `F3` step one CPU cycle
- `F4` step one render/frame 

## Project Layout
- `src/chip8_emulator.*` core VM + opcode implementation
- `src/graphics.*` SDL display and audio 
- `src/debugger.*` debugger functionality
- `src/main.cpp` game loop, orchestration
- `test-roms/` testing ROMs to validate correct instruction handling behaviors
- `game-roms` a few game ROMS to play around with the VM. 
- `fonts/` font TTF(s) for debugger panel + any future rendered text features. 

## Future Plans
- Build a Github Pages/Web deployable setup using WASM etc. Hope to do this soon. 

## Shoutouts & Useful References

[Austin Morlan's chip8_emulator blog](https://austinmorlan.com/posts/chip8_emulator/) : Super detailed notes/guide , heavily referenced the explanations to get started. Implementation isn't perfect but overall really solid piece

-[Timendus's chip8-test-suite repo](https://github.com/Timendus/chip8-test-suite?tab=GPL-3.0-1-ov-file) - used to source the chip-8 test ROMs in this repo and iron out my instruction implementations. Super helpful , esp prior to building the debugger.

-[kripod's chip8-roms repo](https://github.com/kripod/chip8-roms) - great library of ROMs from all sorts of sources , most should work with this setup! There are a few in this repo as well from there. 

[Cowgod's chip-8 technical reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) - an old school tech breakdown of chip-8. This one was great so I wasn't tempted to look at code (only text explanations). Only complaint is a few of the instruction descriptions will get you in trouble later on with the carry flag if not careful. 




