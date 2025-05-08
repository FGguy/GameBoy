#include <GBPlatform.h>
#include <SDL.h>

/*
TODO:
    - Change screen width and height 
    - instanciate cpu bus ppu and other in the constructor
*/

GBPlatform::GBPlatform(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM):
    bootROM{bootROM},
    cartridgeROM{cartridgeROM}
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
