#include <cstdint> 
#include <chrono> 
#include <cstring> 
#include <fstream> 
#include <random> 
#include <stdexcept>

#include "chip8_emulator.hpp"

uint8_t fontset[Chip8System::FONTS_SIZE] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

Chip8System::Chip8System() : rng(std::random_device{}()){
    
    // initialize the Program Counter to the start of loaded program memory block
    program_counter = START_ADDRESS;

    // load fonts into memory in the correct address range
    for(std::size_t i = 0 ; i < FONTS_SIZE; i++){
        memory[FONTS_START_ADDRESS + i] = fontset[i];
    }
    // init function table for dispatching 
    init_tables(); 

}
void Chip8System::init_tables() {
    
    // set all possible dispatches to null command to start so we handle all cases gracefully
    table_master.fill(&Chip8System::op_NULL);
    table_0.fill(&Chip8System::op_NULL);
    table_8.fill(&Chip8System::op_NULL);
    table_E.fill(&Chip8System::op_NULL);
    table_F.fill(&Chip8System::op_NULL);

    // master dispatches
    table_master[0x0]=&Chip8System::Table_0_dispatch;
    table_master[0x1]=&Chip8System::op_1NNN;
    table_master[0x2]=&Chip8System::op_2NNN;
    table_master[0x3]=&Chip8System::op_3XNN;
    table_master[0x4]=&Chip8System::op_4XNN;
    table_master[0x5]=&Chip8System::op_5XY0;
    table_master[0x6]=&Chip8System::op_6XNN;
    table_master[0x7]=&Chip8System::op_7XNN;
    table_master[0x8]=&Chip8System::Table_8_dispatch;
    table_master[0x9]=&Chip8System::op_9XY0;
    table_master[0xA]=&Chip8System::op_ANNN;
    table_master[0xB]=&Chip8System::op_BNNN;
    table_master[0xC]=&Chip8System::op_CXNN;
    table_master[0xD]=&Chip8System::op_DXYN;
    table_master[0xE]=&Chip8System::Table_E_dispatch; 
    table_master[0xF]=&Chip8System::Table_F_dispatch; 
    //0 dispatches
    table_0[0x0]=&Chip8System::op_00E0; 
    table_0[0xE]=&Chip8System::op_00EE;
    //8 dispatches 
    table_8[0x0]=&Chip8System::op_8XY0;
    table_8[0x1]=&Chip8System::op_8XY1;
    table_8[0x2]=&Chip8System::op_8XY2;
    table_8[0x3]=&Chip8System::op_8XY3;
    table_8[0x4]=&Chip8System::op_8XY4;
    table_8[0x5]=&Chip8System::op_8XY5;
    table_8[0x6]=&Chip8System::op_8XY6;
    table_8[0x7]=&Chip8System::op_8XY7;
    table_8[0xE]=&Chip8System::op_8XYE;
    //E dispatches
    table_E[0x1]=&Chip8System::op_EXA1;
    table_E[0xE]=&Chip8System::op_EX9E;
    //F dispatches 
    table_F[0x07]=&Chip8System::op_FX07;
    table_F[0x0A]=&Chip8System::op_FX0A;
    table_F[0x15]=&Chip8System::op_FX15;
    table_F[0x18]=&Chip8System::op_FX18;
    table_F[0x1E]=&Chip8System::op_FX1E;
    table_F[0x29]=&Chip8System::op_FX29;
    table_F[0x33]=&Chip8System::op_FX33;
    table_F[0x55]=&Chip8System::op_FX55;
    table_F[0x65]=&Chip8System::op_FX65;
}

void Chip8System::dispatch(){
    (this->*table_master[(opcode >> 12)])(); 
}
void Chip8System::Table_0_dispatch(){
    (this->*table_0[opcode & 0x000Fu])();
}
void Chip8System::Table_8_dispatch(){
    (this->*table_8[(opcode & 0x000Fu )])();
}
void Chip8System::Table_E_dispatch(){
    (this->*table_E[(opcode & 0x000Fu)])();
}
void Chip8System::Table_F_dispatch(){
    (this->*table_F[(opcode & 0x00FFu)])();
}

void Chip8System::load_ROM(char const* romFilename) {
    std::ifstream ROMContent(romFilename, std::ios::binary | std::ios::ate);  
    if(!ROMContent) throw std::runtime_error("ROM load failed!");

    long unsigned int size =ROMContent.tellg(); // use last position to get size of total content
    ROMContent.seekg(0, std::ios::beg);

    if((size) > (MEMORY_SIZE - START_ADDRESS)) throw std::runtime_error("ROM too large to run!");
    ROMContent.read(reinterpret_cast<char*>(&memory[START_ADDRESS]),size);
}

Chip8System::Debug_snapshot Chip8System::snapshot(){
    // capture debugger metrics for current state 
    Debug_snapshot snap{};
    snap.opcode = opcode;
    snap.pc = program_counter;
    snap.i = index_reg; 
    snap.sp = stack_pointer; 
    snap.dt = delay_timer;
    snap.st = sound_timer; 
    std::copy(std::begin(registers), std::end(registers), snap.registers.begin());
    std::copy(std::begin(stack), std::end(stack), snap.stack.begin());
    std::copy(std::begin(memory), std::end(memory), snap.memory.begin());
    return snap; 
}

void Chip8System::reset(){
    // reinitialize the system , essentially wipe the memory and re-costruct but with
    // existing instance
  opcode = 0;
    std::fill(std::begin(memory), std::end(memory), 0);
    std::fill(std::begin(registers), std::end(registers), 0);
    std::fill(std::begin(stack), std::end(stack), 0);
    std::fill(std::begin(display), std::end(display), 0);
    std::fill(std::begin(keys), std::end(keys), 0);
    std::fill(std::begin(just_pressed), std::end(just_pressed), 0);
    std::fill(std::begin(just_released), std::end(just_released), 0);
    stack_pointer = 0;
    index_reg = 0;
    program_counter = START_ADDRESS;
    delay_timer = 0;
    sound_timer = 0;
    awaiting_input = false;
    awaiting_release = false;
    wait_reg = 0;
    down_key = 0;
    for (std::size_t i = 0; i < FONTS_SIZE; ++i) {
        memory[FONTS_START_ADDRESS + i] = fontset[i];
    }
    init_tables();
}

bool Chip8System::sound_active(){
    return sound_timer > 0; 
}

void Chip8System::op_NULL(){
    // do nothing, this is a bad dispatch path (invalid command)
}

void Chip8System::op_0NNN(){
    return; // dud command , we never use this one 
}

void Chip8System::op_00E0(){
    std::fill(std::begin(display),std::end(display),0);
} 

void Chip8System::op_00EE(){
    stack_pointer--; 
    program_counter = stack[stack_pointer]; 
}

void Chip8System::op_1NNN(){
    uint16_t address = opcode & 0x0FFFu; // mask out the command bits
    program_counter = address; 
} 

void Chip8System::op_2NNN(){
    uint16_t address = opcode & 0x0FFFu;
    stack[stack_pointer] = program_counter; // store current pc for subroutine return
    ++stack_pointer;
    program_counter = address; 
} 

void Chip8System::op_3XNN(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // mask non register bits, then shift into proper place 
    uint8_t NN = opcode & 0x00FFu; 
    if( registers[Vx] == NN ) program_counter += 2; // increment by 2 to skip an instruction 
} 

void Chip8System::op_4XNN(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; 
    uint8_t NN = opcode & 0x00FFu;
    if( registers[Vx] != NN ) program_counter += 2;
}

void Chip8System::op_5XY0(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    if(registers[Vx] == registers[Vy]) program_counter += 2; 
}

void Chip8System::op_6XNN(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; 
    uint8_t NN = opcode & 0x00FFu;
    registers[Vx] = NN; 

}

void Chip8System::op_7XNN(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; 
    uint8_t NN = opcode & 0x00FFu;
    registers[Vx] += NN; 
} 

void Chip8System::op_8XY0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    registers[Vx] = registers[Vy]; 
}

void Chip8System::op_8XY1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    registers[Vx] |= registers[Vy];
    registers[0xF] = 0; 
} 

void Chip8System::op_8XY2(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    registers[Vx] &= registers[Vy]; 
    registers[0xF] = 0;
}

void Chip8System::op_8XY3(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    registers[Vx] ^= registers[Vy];
    registers[0xF] = 0;
}

void Chip8System::op_8XY4(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    uint16_t ret = registers[Vx] + registers[Vy]; 
    registers[Vx] = ret & 0x00FFu;
    if(ret > 255u) registers[0xF] = 1; // if result of addition is greater than 8 bits , set overflow flag in VF reg
    else registers[0xF] = 0; 
} 

void Chip8System::op_8XY5(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    bool flag = registers[Vx] >= registers[Vy]; 
    registers[Vx] -= registers[Vy];
    if(flag) registers[0xF] = 1; 
    else registers[0xF] = 0; 
}

void Chip8System::op_8XY6(){
      uint8_t Vx = (opcode & 0x0F00u) >>8u; 
      uint16_t flag = (registers[Vx] & 0x001u);
      registers[Vx] >>= 1 ; 
      registers[0xF] = flag;
}

void Chip8System::op_8XY7(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    bool flag = (registers[Vy] >= registers[Vx]);
    registers[Vx] = registers[Vy] - registers[Vx];
    if(flag) registers[0xF] = 1; 
    else registers[0xF] = 0;
    
}

void Chip8System::op_8XYE(){
      uint8_t Vx = (opcode & 0x0F00u) >>8u; 
      uint16_t flag = (registers[Vx] & 0x80u) >> 7u;
      registers[Vx] <<= 1 ; 
      registers[0xF] = flag;
}

void Chip8System::op_9XY0(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    if(registers[Vx] != registers[Vy]) program_counter += 2; 
} 

void Chip8System::op_ANNN(){
    uint16_t addr = opcode & 0x0FFFu; 
    index_reg = addr; 
} 

void Chip8System::op_BNNN(){
    uint16_t addr = opcode & 0x0FFFu;
    program_counter = registers[0] + addr;  
} 

void Chip8System::op_CXNN(){
     uint8_t Vx = (opcode & 0x0F00u) >>8u; 
     uint8_t NN = opcode & 0x00FFu; 

     uint8_t random_value = byte_dist(rng);
     registers[Vx] = random_value & NN; 
}

void Chip8System::op_DXYN(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u; 
    uint8_t Vy = (opcode & 0x00F0u) >>4u;
    uint8_t height = opcode & 0x000Fu; 
    
    // handle wrapping when start goes over screen boundaries
    const uint8_t X_START = registers[Vx] % VIDEO_W; 
    const uint8_t Y_START = registers[Vy] % VIDEO_H; 
    registers[0xF] = 0; 
    
    // iterate over each sprite row to build n height (byte)
    for(unsigned int i = 0 ; i < height ; ++i){
        const uint8_t sprite_block = memory[index_reg + i]; 
        const uint8_t y = (Y_START + i) % VIDEO_H; // wrap by pixel
        
        // iterate over each pixel in the row
        for(uint8_t j = 0 ; j < 8 ; ++j){
            const bool isOn = (sprite_block & (0x80u >> j)) != 0; 
            if(!isOn) continue; 
            const uint8_t x = (X_START + j) % VIDEO_W;
            uint32_t& pixel = display[y * VIDEO_W + x]; 
            if(pixel == 0xFFFFFFFFu) registers[0xF] = 1 ; // set collision
            pixel ^= 0xFFFFFFFFu; // XOR to flip the current state 
        }
    }
}

void Chip8System::op_EX9E(){
     uint8_t Vx = (opcode & 0x0F00u) >>8u;
     uint8_t key = registers[Vx]; 
     if(keys[key]) program_counter += 2 ; 
}

void Chip8System::op_EXA1(){
     uint8_t Vx = (opcode & 0x0F00u) >>8u;
     uint8_t key = registers[Vx]; 
     if(!keys[key]) program_counter += 2 ; 
}

void Chip8System::op_FX07(){
     uint8_t Vx = (opcode & 0x0F00u) >>8u;
     registers[Vx] = delay_timer;
}

void Chip8System::op_FX0A(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;
    bool found = false; 
    for(uint8_t i = 0 ; i < 16; ++i){
        if(just_pressed[i]) {
            registers[Vx] = i;
            found = true; 
        }
    }
    // if no input then set the waiting flag to stall cpu 
    if(!found){
        awaiting_input = true; 
        wait_reg = Vx;
    }
}

void Chip8System::op_FX15(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;
    delay_timer = registers[Vx]; 
} 

void Chip8System::op_FX18(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;
    sound_timer = registers[Vx];
}

void Chip8System::op_FX1E(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u; 
    index_reg += registers[Vx]; 
}

void Chip8System::op_FX29(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;
    uint8_t dig = registers[Vx]; 
    // use font start and offset to find first byte of target
    index_reg = FONTS_START_ADDRESS + (5*dig); 
}

void Chip8System::op_FX33(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;
    uint8_t val = registers[Vx]; 
    memory[index_reg + 2] = val % 10;
    val /= 10; 
    memory[index_reg + 1] = val % 10;
    val /= 10;
    memory[index_reg] = val % 10; 
    val /=10; 
}

void Chip8System::op_FX55(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;
    for(uint8_t i = 0 ; i <= Vx; ++i){
        memory[index_reg + i] = registers[i]; 
    }
}

void Chip8System::op_FX65(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for(uint8_t i = 0 ; i <= Vx ; ++i){
        registers[i] = memory[index_reg + i]; 
    }
}

void Chip8System::cycle() {
    // handle suspension state for op_FX0A
    if(awaiting_input) {
        for(uint8_t i = 0 ; i < REGISTERS; ++i){
            if(just_pressed[i]) {
                awaiting_input = false;
                awaiting_release = true;  
                down_key = i; 
            }
        }  
    }
    else if(awaiting_release){
       if(just_released[down_key]){
            registers[wait_reg] = down_key;
            down_key = 0;
            wait_reg = 0; 
            awaiting_release = false; 
       }
    }
    else{
        // 2 8-bit addresses to 16-bit instruction
        opcode = (memory[program_counter] << 8u | memory[program_counter+1]); 
        program_counter += 2 ;
        dispatch();
    }
}

void Chip8System::tick_timers() {
    if(delay_timer > 0) --delay_timer;
    if(sound_timer > 0) --sound_timer;
}



