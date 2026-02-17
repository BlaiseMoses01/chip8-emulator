#include "chip8_emulator.hpp"
#include "graphics.hpp"
#include "debugger.hpp"
#include <chrono>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom>" << std::endl;
        return 1;
    }

    Chip8System chip8;
    Debug debugger; 
    Graphics gfx;

    if(!gfx.init("CHIP-8", 12)) {
        std::cerr << "Failed to initialize graphics" << std::endl;
        return 1;
    }

    try {
        chip8.load_ROM(argv[1]);
    } catch(const std::exception& ex) {
        std::cerr << "Failed to load ROM: " << ex.what() << std::endl;
        gfx.shutdown();
        return 1;
    }

    constexpr double CPU_HZ = 700.0;
    constexpr double TIMER_HZ = 60.0;
    constexpr double CPU_STEP = 1.0 / CPU_HZ;
    constexpr double TIMER_STEP = 1.0 / TIMER_HZ;

    bool running = true;
    double cpu_acc = 0.0;
    double timer_acc = 0.0;
    auto last_time = std::chrono::steady_clock::now();
    bool show_debug = false; 

    while(running) {
        const auto now = std::chrono::steady_clock::now();
        const double dt = std::chrono::duration<double>(now - last_time).count();
        last_time = now;

        cpu_acc += dt;
        timer_acc += dt;

        
        Graphics::Debug_input d{}; // pass debugger to collect debug state  from user 
        running = gfx.process_input(chip8.keys, chip8.just_pressed, chip8.just_released,d);

        if(d.show_debug) show_debug = !show_debug; 
        if (d.flip_mode) debugger.flip_mode();
        if (d.step_one_cycle) debugger.step_one_cycle();
        if (d.step_one_render) debugger.step_one_render();
        

        while (cpu_acc >= CPU_STEP) {
            if (debugger.can_execute_cycle()) {
                chip8.cycle();
            }
            cpu_acc -= CPU_STEP;
        }

        while (timer_acc >= TIMER_STEP) {
            if (debugger.can_tick_timers()) {
                chip8.tick_timers();
                gfx.set_playback(chip8.sound_active());
            }
            timer_acc -= TIMER_STEP;
        }

        Chip8System::Debug_snapshot snapshot{}; 
        const Chip8System::Debug_snapshot* snapshot_ptr; 
        if(show_debug) {
            snapshot = chip8.snapshot(); 
            snapshot_ptr = &snapshot;
        }

        gfx.render(chip8.display, snapshot_ptr, debugger.current_mode(), show_debug);
        debugger.on_frame_presented();
    }
    gfx.shutdown();
    return 0;
}
