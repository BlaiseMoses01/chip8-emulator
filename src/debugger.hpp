#pragma once 

#include<cstdint>

class Debug {
    public: 
        enum class Mode : uint8_t {
            Running, 
            Paused 
        };
        void flip_mode(); 
        void step_one_cycle(); 
        void step_one_render();
        bool can_execute_cycle();
        bool can_tick_timers(); 
        void on_frame_presented(); 
        Mode current_mode(){return mode;}
    private:
        Mode mode{Mode::Running};
        uint32_t cycle_budget{0};
        bool pause_upon_present{false};
};