# CHIP-8 Dev Log

Running journal of my additions , debugging , and general lessons learned from building this project.

## Milestone 1: Core VM Setup

I started with the core CHIP-8 architecture:
- 4KB memory
- 16 general-purpose registers
- stack + stack pointer
- index register and program counter
- delay and sound timers
- 64x32 framebuffer

### Noteable Design Lessons 
- Use `static constexpr` constants for class-wide values, value is available at compile-time versus const where it could get initialized at runtime. 

## Milestone 2: Opcode Decode + Dispatch

A lot of people tend to just use a switch statement to handle all the codes. This works fine for a small instruction set like chip8 , but I eventually want to try and emulate some more complex consoles ,so  I wanted to do the right approach 

I built a dispatch-table based decoder (`table_master`, `table_0`, `table_8`, `table_E`, `table_F`) so opcodes route to instruction handlers cleanly.

This isn't super complex once you get it , but it did kind of throw me off for a bit since my pointer logic etc was a bit rusty. I eventually got it though.

Essentially you bucket opcodes into uniqueness based on nibbles , and then at each level if you can distinguish one command,  you dispatch it , otherwise you dispatch a nested dispatch table , and it tries to distinguish the same way at the next level, so on and so forth. 

### Notable Bugs
- I initially used `uint8_t` in some places for the `ANNN` and `BNNN` paths.That truncated the 12-bit addresses (`0x0FFF` mask), which caused wrong memory targets and made the fonts showing up instead of expected ROM behavior when I ran the test roms.

- **Solution** : Changed address intermediates to `uint16_t` where required.

### Notable Design Lessons
- Great review of pointers and bitwise operations. Most of this required working with hexadecimal and binary and writing masks intutively.

## Milestone 3: Math/Flag Correctness

Arithmetic/logic tests initially failed in edge cases involving carry/borrow state.

### Notable Bugs
- I was setting the flag register `VF` pre-op in my math operations, which creates an edge case bug when the current state of VF is one of the operands of the desired math function. 

- **Solution**: I added intermediates to store pre-op `VF` desired state, then set `VF` post-op to ensure proper operation sequencing
- 

## Notable Design Lessons 
- Good thought exercise on state management and overflow. 

## Milestone 4: Input optimization, handling key edge events 

### Notable bug
`FX0A` opcode failed on "press and release" behavior in keypad ROM mode 3 .

In my initial build, I implemented `FX0A` to only handle the overall key state, up or down , or in other words a hold event.

However I neglected to account for the edge key behavior like a press and release , which is more appropriate for this command , since it pauses and resumes upon a key being hit. 

-**Solution**: added edge event arrays to store a press and release in addition to the original overall key state array. Then updated SDL loop to populate the input edge states there and the CPU's wait for input logic to account for the edge events. 


## Milestone 5: Audio (Sound Timer Beep)

Audio took more effort than expected. I was eventually able to get it implemented but ended up chatting with Codex for some high level guidance around the different SDL2 tools to use, since the documentation is less than straightforward. I also had it setup some of the more granular audio stuff like the waveform.

The saving grace is Chip8 only had a single tone beep audio tone, so it was pretty lightweight to get working. Eventually I got the callback-based audio setup working ,and added logic to connect the SDL setup to the VM to drive audio events off of the sound timer. 

## Milestone 6: Quirk Behavior Reality Check

After getting all the other chip8 test ROMs working , I ran quirks tests and found that my core is currently a hybrid of classic CHIP-8 and later CHIP-48/SCHIP-style behavior.

I initially tried to modify my instruction set to use strictly original chip8 style logic, but this actually broke a few of my target ROMs , so I ended up deciding to stay with my hybrid setup. 

It seems to work well with everything I've ran thus far, but I'm sure theres some older ROMs out there that might give it trouble. 

## Milestone 7: Debugger Controls + Overlay

I added debugger controls and in-app debug display support:
- `F1` run/pause
- `F2` step one cycle
- `F3` step one frame
- `F4` toggle overlay

This actually wasn't super difficult, despite it sounding like a complex task on the surface. I just wrote a struct for all of the memory/system metrics I wanted to display , a function to capture their current values from the VM instance at any given state, and then tied it in with the SDL setup. I definitely did some googling and had codex give me a bit of the SDL stuff here ,as I ended up having to use SDL_ttf to render a font etc and this was a bit outside of my interest. 

I was super pleased with the result though , the panel is super cool and its nice to be able to pause and step through ROM's if needed. For this project prob won't use much other than playing around , but for future , larger em's , will def be adding something like this early on as its super insightful. 

