#include "SDLGameBoyPlatformLayer.h"

#include <stdexcept>

SDLGameBoyPlatformLayer::SDLGameBoyPlatformLayer() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        throw std::runtime_error("Failed to initialize SDL");
    }

    gbWindow = SDL_CreateWindow("GameBoy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 640, SDL_WINDOW_SHOWN);
    if (!gbWindow) {
        SDL_Quit();
        throw std::runtime_error("Failed creating SDL Window");
    }

    gbRenderer = SDL_CreateRenderer(gbWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gbRenderer) {
        SDL_DestroyWindow(gbWindow);
        SDL_Quit();
        throw std::runtime_error("Failed creating SDL Renderer");
    }

    gbScreenTexture = SDL_CreateTexture(
        gbRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        160,
        144
    );

    if (!gbScreenTexture) {
        SDL_Log("Texture creation failed: %s", SDL_GetError());
        SDL_DestroyRenderer(gbRenderer);
        SDL_DestroyWindow(gbWindow);
        SDL_Quit();
    }
}

void SDLGameBoyPlatformLayer::processInputs(InputState& state){
 SDL_Event event;
    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_QUIT: {
                state.quit = true;
            } break;
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym){
                    case SDLK_a:
                        state.b_left = 0;
                        break;
                    case SDLK_w:
                        state.b_up = 0;
                        break;
                    case SDLK_s:
                        state.b_down = 0;
                        break;
                    case SDLK_d:
                        state.b_right = 0;
                        break;
                    case SDLK_j:
                        state.b_B = 0;
                        break;
                    case SDLK_k:
                        state.b_A = 0;
                        break;
                    case SDLK_v:
                        state.b_select = 0;
                        break;
                    case SDLK_b:
                        state.b_start = 0;
                        break;
                }
                state.joypadInterruptRequested = true;
            } break;
            case SDL_KEYUP: {
                switch (event.key.keysym.sym){
                    case SDLK_a:
                        state.b_left = 1;
                        break;
                    case SDLK_w:
                        state.b_up = 1;
                        break;
                    case SDLK_s:
                        state.b_down = 1;
                        break;
                    case SDLK_d:
                        state.b_right = 1;
                        break;
                    case SDLK_j:
                        state.b_B = 1;
                        break;
                    case SDLK_k:
                        state.b_A = 1;
                        break;
                    case SDLK_v:
                        state.b_select = 1;
                        break;
                    case SDLK_b:
                        state.b_start = 1;
                        break;
                }
            } break;
        }
    }
}

void SDLGameBoyPlatformLayer::renderFrame(std::vector<Pixel> display_buffer) {
    // Define Game Boy color palette (classic green tint)
    const uint32_t palette[4] = {
        0xFFE0F8D0,  // White/Lightest
        0xFF88C070,  // Light Gray/Light Green
        0xFF346856,  // Dark Gray/Dark Green
        0xFF081820   // Black/Darkest
    };

    // Lock texture for pixel manipulation
    void* texture_pixels;
    int pitch;
    if (SDL_LockTexture(this->gbScreenTexture, nullptr, &texture_pixels, &pitch) != 0) {
        SDL_Log("Failed to lock texture: %s", SDL_GetError());
        return;
    }

    // Copy pixels from display buffer to texture
    uint32_t* pixel_data = static_cast<uint32_t*>(texture_pixels);

    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            uint8_t color_index = display_buffer[y * 160 + x].color;
            pixel_data[y * (pitch / 4) + x] = palette[color_index & 0x03];
        }
    }

    // Unlock texture
    SDL_UnlockTexture(this->gbScreenTexture);

    // Clear renderer
    SDL_SetRenderDrawColor(this->gbRenderer, 0, 0, 0, 255);
    SDL_RenderClear(this->gbRenderer);

    // Copy texture to renderer
    SDL_RenderCopy(this->gbRenderer, this->gbScreenTexture, nullptr, nullptr);

    // Present the frame
    SDL_RenderPresent(this->gbRenderer);
}

SDLGameBoyPlatformLayer::~SDLGameBoyPlatformLayer() {
    SDL_DestroyRenderer(gbRenderer);
    SDL_DestroyWindow(gbWindow);
    SDL_Quit();
}


