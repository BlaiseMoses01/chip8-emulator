#include "debugger.hpp"

void Debug::flip_mode(){
    if(mode == Mode::Paused) mode = Mode::Running;
    else mode = Mode::Paused; 

    cycle_budget = 0;
    pause_upon_present = false; 
}

void Debug::step_one_cycle() {
    mode = Mode::Paused; 
    cycle_budget = 1; 
    pause_upon_present = false; 
}

void Debug::step_one_render(){
    mode = Mode::Running; 
    cycle_budget = 0; 
    pause_upon_present = true; 
}
bool Debug::can_execute_cycle(){
    if(mode == Mode::Running) return true; 
    
    if(cycle_budget > 0) {
        --cycle_budget;    
        return true;
    } 
    return false; 
}

bool Debug::can_tick_timers(){
    return mode == Mode::Running;
}

void Debug::on_frame_presented(){
    if(pause_upon_present){
        mode = Mode::Paused; 
        pause_upon_present = false; 
    }
}