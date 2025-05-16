#include <GBPlatform.h>

/*
TODO:
    - Change screen width and height 
    - instanciate cpu bus ppu and other in the constructor
*/

GBPlatform::GBPlatform(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM):
    bootROM{bootROM},
    cartridgeROM{cartridgeROM},
    gbBus{bootROM, cartridgeROM}
{
}

int GBPlatform::BootAndExecute(){
    //start window setup
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { //add error message if fails
        return 1;
    }

    gbWindow = SDL_CreateWindow("GameBoy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 640, SDL_WINDOW_SHOWN);
    if (!gbWindow) {
        SDL_Quit();
        return 1;
    }

    gbRenderer = SDL_CreateRenderer(gbWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gbRenderer) {
        SDL_DestroyWindow(gbWindow);
        SDL_Quit();
        return 1;
    }


    //cleanup
    SDL_DestroyRenderer(gbRenderer);
    SDL_DestroyWindow(gbWindow);
    SDL_Quit();
    return 0;
}

void GBPlatform::ProcessInputs(){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_QUIT: {
                //TODO make the program quit
            } break;
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym){
                    case SDLK_a:
                        b_left = 0;
                        break;
                    case SDLK_w:
                        b_up = 0;
                        break;
                    case SDLK_s:
                        b_down = 0;
                        break;
                    case SDLK_d:
                        b_right = 0;
                        break;
                    case SDLK_j:
                        b_B = 0;
                        break;
                    case SDLK_k:
                        b_A = 0;
                        break;
                    case SDLK_v:
                        b_select = 0;
                        break;
                    case SDLK_b:
                        b_start = 0;
                        break;
                }
            } break;
            case SDL_KEYUP: {
                switch (event.key.keysym.sym){
                    case SDLK_a:
                        b_left = 1;
                        break;
                    case SDLK_w:
                        b_up = 1;
                        break;
                    case SDLK_s:
                        b_down = 1;
                        break;
                    case SDLK_d:
                        b_right = 1;
                        break;
                    case SDLK_j:
                        b_B = 1;
                        break;
                    case SDLK_k:
                        b_A = 1;
                        break;
                    case SDLK_v:
                        b_select = 1;
                        break;
                    case SDLK_b:
                        b_start = 1;
                        break;
                }
            } break;
        }
    }
}