#pragma once 

#include <cstdint> 
#include <random>
#include <array>

class Chip8System {
    public: 
        static constexpr std::size_t INPUT_SIZE = 16;  // chip 8 uses 16 input keys corresponding to the first 16 hex values 0 - F(15)
        static constexpr std::size_t MEMORY_SIZE = 4096; // chip 8 has 4k memory , so 4096 bytes
        static constexpr std::size_t REGISTERS = 16; // 16 registers in original design
        static constexpr std::size_t STACK_SIZE = 16; 
        static constexpr std::size_t VIDEO_H = 32; 
        static constexpr std::size_t VIDEO_W = 64;  
        static constexpr std::size_t FONTS_SIZE = 80; 
        static constexpr std::size_t F_TABLE_SIZE = 256; // F table needs full byte to distinguish commands
        static constexpr std::size_t FONTS_START_ADDRESS = 0x050 ; // font set was originally stored 0x050 - 0x0A0
        static constexpr std::size_t START_ADDRESS = 0x200;

        uint32_t display[VIDEO_W * VIDEO_H]{}; // display window 64 x 32 pixels (32 bit pixels) 
        uint8_t keys[REGISTERS]{};
        uint8_t just_pressed[REGISTERS]{};
        uint8_t just_released[REGISTERS]{};
        uint8_t down_key{};

        struct Debug_snapshot {
        uint16_t opcode{};  
        uint8_t sp{}; 
        uint16_t i{}; 
        uint16_t pc{}; 
        uint8_t dt{};
        uint8_t st{}; 
        std::array<uint8_t, REGISTERS> registers{};
        std::array<uint16_t, STACK_SIZE> stack{};
        std::array<uint8_t, MEMORY_SIZE> memory{};
        };
        
        Chip8System();
        void load_ROM(const char* path); 
        void cycle();
        void tick_timers();
        bool sound_active(); 
        
        Debug_snapshot snapshot(); 
        void reset(); 


    private:
        uint16_t opcode; 
        uint8_t memory[MEMORY_SIZE]{};
        uint8_t registers[REGISTERS]{}; 
        uint16_t stack[STACK_SIZE]{};    
        uint8_t stack_pointer{}; 
        uint16_t index_reg{}; // index register stores memory addresses for operations
        uint16_t program_counter{}; // program counter register stores the next instruction to execute
        uint8_t delay_timer{};
        uint8_t sound_timer{}; 
        std::mt19937 rng; 
        std::uniform_int_distribution<uint8_t> byte_dist{0,255};
        bool awaiting_input{false};
        bool awaiting_release{false};
        uint8_t wait_reg{0}; 
        
        using Chip8Func = void(Chip8System::*)();
        
        //dispatch tables
        std::array<Chip8Func, INPUT_SIZE> table_master{}; 
        std::array<Chip8Func, INPUT_SIZE> table_0{};
        std::array<Chip8Func, INPUT_SIZE> table_8{};
        std::array<Chip8Func, INPUT_SIZE> table_E{};
        std::array<Chip8Func, F_TABLE_SIZE> table_F{}; 
        
        
        //dispatch functions
        void init_tables();
        void dispatch(); 
        void Table_0_dispatch();
        void Table_8_dispatch();
        void Table_E_dispatch();
        void Table_F_dispatch();

        //opcode functions
        void op_NULL(); // dead op for invalid instructions
        void op_0NNN(); // machine code routine command ( not needed for our purposes, but here for coverage)
        void op_00E0(); // CLS: clear the display
        void op_00EE(); // RET: returns from a subroutine
        void op_1NNN(); // JP addr: jumps to location NNN
        void op_2NNN(); // CALL addr: calls subroutine at NNN  
        void op_3XNN(); // SE Vx, byte : skip next instruction if Vx == NN
        void op_4XNN(); // SNE Vx, byte : skip next instruction if Vx != NN
        void op_5XY0(); // SE Vx, Vy : skip next instruction if Vx == Vy 
        void op_6XNN(); // LD Vx , byte : place value NN into register Vx
        void op_7XNN(); // ADD Vx, byte  : adds value NN to value of register Vx , stores result in Vx
        void op_8XY0(); // LD Vx, Vy : stores value of register Vy in register Vx
        void op_8XY1(); // OR Vx, vy : bitwise OR on the values of Vx and Vy , store result in Vx 
        void op_8XY2(); // AND Vx, Vy : bitwise AND on values of Vx and Vy, store result in Vx 
        void op_8XY3(); // XOR Vx,Vy : bitwise exclusive OR on the values of Vx and Vy, store result in Vx 
        void op_8XY4(); // ADD Vx,Vy : Vx = Vx + Vy, VF = cary
        void op_8XY5(); // SUB Vx,Vy : Vx = Vx - Vy , set VF = NOT borrow. Vx > Vy , VF = 1 , else 0. 
        void op_8XY6(); // SHR Vx {, Vy} :  least significant bit == 1 then VF = 1 , else 0 , Vx = Vx shift right 1 
        void op_8XY7(); // SUBN Vx Vy : Vx = Vy - Vx , set VF = Not borrow 
        void op_8XYE(); // SHL Vx  {, Vy} : Vx = Vx shift left 1
        void op_9XY0(); // SNE Vx Vy : skip next instruction if Vx != Vy 
        void op_ANNN(); // LD I , addr Set I = nnn 
        void op_BNNN(); // P V0 , addr : PC = nnn + V0
        void op_CXNN(); // RND Vx, byte Vx = random byte AND NN 
        void op_DXYN(); // DRW Vx , Vy , nibble : draws sprites for the display 
        void op_EX9E(); // SKP Vx : skip next instruction if the key with value Vx is pressed
        void op_EXA1(); // SKNP Vx : sip next instructio nif the key with value Vx is not pressed
        void op_FX07(); // LD Vx , Dt : Set Vx = delay timer value 
        void op_FX0A(); // LD Vx, k : wait for a key press and store the key value in Vx . Pauses execution
        void op_FX15(); // LD DT,  Vx : delay timer = Vx 
        void op_FX18(); // LD ST, Vx : sound timer = Vx 
        void op_FX1E(); // Add I , Vx :  I = I + Vx I = I + Vx 
        void op_FX29(); // LD F , Vx : I = sprite location digit Vx 
        void op_FX33(); // LD B, Vx : store BCD rep of Vx in memory locations I, I+1, I+2 
        void op_FX55(); // LD [I] , Vx : Store rgisters V0 through Vx in meory starting at location I 
        void op_FX65(); // LD vx, [i] : Read registers V0 thoruhg Vx from memory starting at location I 
};
