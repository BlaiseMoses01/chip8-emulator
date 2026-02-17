#pragma once 

#include <SDL2/SDL.h> 
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <atomic>
#include <string>

#include "debugger.hpp"
#include "chip8_emulator.hpp"

class Graphics{
    public:
        static constexpr int WIDTH = 64;
        static constexpr int HEIGHT = 32; 
        static constexpr const char* DEBUG_FONT = "fonts/OCRAExt.TTF";
        struct Debug_input{
            bool flip_mode{false};
            bool step_one_cycle{false};
            bool step_one_render{false};
            bool show_debug{false}; 
        };

        bool init(const char* title, int scale);
        bool process_input(uint8_t keys[16], uint8_t just_pressed[16] , uint8_t just_released[16], Debug_input& dbg); 
        void shutdown();
        void render(const uint32_t* framebuffer, const Chip8System::Debug_snapshot* snapshot, Debug::Mode mode, bool show_debug); 
        void set_playback(bool enabled);
    private: 
        float phase{0.0f};
        float frequency{440.0f};
        float volume{0.20f}; 
        std::atomic<bool> playback_on{false};
    
        SDL_Window* window{nullptr};
        SDL_Renderer* renderer{nullptr};
        SDL_Texture* texture{nullptr}; 
        SDL_AudioDeviceID audio_device{0}; // audio device 
        SDL_AudioSpec audio_spec{}; // format (int channels: 1 mono, 2 stereo, etc, int freq : sample rate)

        static void audio_callback(void* userdata, Uint8* stream, int len);
        void draw_debug_text(int x , int y , const std::string& text);
        TTF_Font* debug_font{nullptr}; // debugger font handler
};
