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

void SDLGameBoyPlatformLayer::renderFrame() {
    //TODO: Implement rendering logic
}

SDLGameBoyPlatformLayer::~SDLGameBoyPlatformLayer() {
    SDL_DestroyRenderer(gbRenderer);
    SDL_DestroyWindow(gbWindow);
    SDL_Quit();
}


