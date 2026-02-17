#include <iostream> 
#include <cstring>
#include <sstream>
#include <iomanip> 

#include "graphics.hpp"


static int getKeyMapping(SDL_Keycode key) {
    switch (key) {
        case SDLK_1: return 0x1; 
        case SDLK_2: return 0x2; 
        case SDLK_3: return 0x3; 
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4; 
        case SDLK_w: return 0x5; 
        case SDLK_e: return 0x6; 
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7; 
        case SDLK_s: return 0x8; 
        case SDLK_d: return 0x9; 
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA; 
        case SDLK_x: return 0x0; 
        case SDLK_c: return 0xB; 
        case SDLK_v: return 0xF;
        default: return -1;
    }
}

void Graphics::audio_callback(void* userdata, Uint8* stream, int len) {
    auto* self = static_cast<Graphics*>(userdata);
    auto* out = reinterpret_cast<float*>(stream);
    const int samples = len / static_cast<int>(sizeof(float));

    if (!self || !self->playback_on.load(std::memory_order_relaxed)) {
        std::memset(stream, 0, len);
        return;
    }

    const float sample_rate = static_cast<float>(self->audio_spec.freq);

    for (int i = 0; i < samples; ++i) {
        out[i] = (self->phase < 0.5f ? 1.0f : -1.0f) * self->volume;
        self->phase += self->frequency / sample_rate;
        if (self->phase >= 1.0f) self->phase -= 1.0f;
    }
}

bool Graphics::init(const char* title, int scale) {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Failed to init SDL" << SDL_GetError() << std::endl;
        return false; 
    }
    window = SDL_CreateWindow(title, 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        WIDTH * scale, HEIGHT *scale,
         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if(!window) {
        std::cerr << "failed to create SDL window" << SDL_GetError() << std::endl;
        return false; 
    }

    renderer = SDL_CreateRenderer(
        window, 
        -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
     if (!renderer) {
        std::cerr << "renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // nearest-neighbor texture scaling 
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );
    if(!texture) {
        std::cerr << "texture creation failed" << SDL_GetError() <<std::endl;
        return false; 
    }
    SDL_AudioSpec want{};
    want.freq = 48000;
    want.format = AUDIO_F32SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = &Graphics::audio_callback;
    want.userdata = this;

    audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, &audio_spec, 0);
    if (audio_device == 0) {
        std::cerr << "Audio disabled: " << SDL_GetError() << std::endl;
    } else {
        SDL_PauseAudioDevice(audio_device, 0);
    } 

    // Debugger text handling 
    if (TTF_Init() != 0) {
        std::cerr << "TTF init failed: " << TTF_GetError() << std::endl;
        return false;
    }
    debug_font = TTF_OpenFont(DEBUG_FONT, 14);
    if (!debug_font) {
        std::cerr << "Font load failed: " << TTF_GetError() << std::endl;
    }
    return true; 
}

void Graphics::draw_debug_text(int x, int y, const std::string& text) {
    if (!debug_font) return;
    SDL_Color fg{240, 240, 240, 255};
    SDL_Surface* s = TTF_RenderUTF8_Blended(debug_font, text.c_str(), fg);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_Rect r{x, y, s->w, s->h};
    SDL_FreeSurface(s);
    if (!t) return;
    SDL_RenderCopy(renderer, t, nullptr, &r);
    SDL_DestroyTexture(t);
}
 


void Graphics::render(const uint32_t* framebuffer,
    const Chip8System::Debug_snapshot* snapshot,
    Debug::Mode mode, 
    bool show_debug
){
    SDL_UpdateTexture(texture, nullptr, framebuffer, WIDTH * static_cast<int>(sizeof(uint32_t)));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer,texture,nullptr,nullptr);
    
    if(show_debug && snapshot){
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer,0,0,0,170);
        SDL_Rect debug_panel{8,8,360,170};
        SDL_RenderFillRect(renderer, &debug_panel);
           std::ostringstream line1;
        line1 << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << snapshot->pc
              << "  OP: 0x" << std::setw(4) << snapshot->opcode
              << "  I: 0x" << std::setw(4) << snapshot->i;
        draw_debug_text(16, 16, line1.str());

        std::ostringstream line2;
        line2 << "DT: " << std::dec << (int)snapshot->dt
              << "  ST: " << (int)snapshot->st
              << "  SP: " << (int)snapshot->sp
              << "  MODE: " << (mode == Debug::Mode::Running ? "RUN" : "PAUSE");
        draw_debug_text(16, 40, line2.str());

        for (int i = 0; i < 16; ++i) {
            std::ostringstream vr;
            vr << "V" << std::hex << std::uppercase << i
               << ": " << std::setw(2) << std::setfill('0')
               << (int)snapshot->registers[i];
            int col = i / 8;
            int row = i % 8;
            draw_debug_text(16 + col * 160, 68 + row * 16, vr.str());
        }      
    }
    SDL_RenderPresent(renderer);
}

bool Graphics::process_input(uint8_t keys[16], 
    uint8_t just_pressed[16] , 
    uint8_t just_released[16],
    Graphics::Debug_input& d
){
    SDL_Event e; 
    d = {}; 
    // zero edge event arrays to prevent stale events
    for(int i = 0 ; i < 16; ++i){
        just_pressed[i] = 0 ;
        just_released[i] = 0; 
    }
    
    while(SDL_PollEvent(&e)){
        if(e.type == SDL_QUIT) return false; //exit event
        
        // handle esc quit and debug keys
        if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE: return false;
                case SDLK_F1:  d.show_debug = true; break;
                case SDLK_F2:  d.flip_mode = true; break;
                case SDLK_F3: d.step_one_cycle= true; break;
                case SDLK_F4: d.step_one_render = true; break;
            }
        } 
        
        if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            int key = getKeyMapping(e.key.keysym.sym);
            if(key >= 0){
                if(e.type == SDL_KEYDOWN && e.key.repeat==0) {
                    keys[key] = 1; // overall key state
                    just_pressed[key] = 1; //edge state (pos)
                }
                if(e.type == SDL_KEYUP){
                    keys[key] = 0;
                    just_released[key] = 1; //edge state (neg)
                }
            }
        }
    }
    return true;
}

void Graphics::shutdown(){
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (audio_device) SDL_CloseAudioDevice(audio_device);
    texture = nullptr;
    renderer = nullptr;
    window = nullptr;
    audio_device = 0; 
    SDL_Quit();
}
void Graphics::set_playback(bool enabled){
    playback_on.store(enabled,std::memory_order_relaxed);
}