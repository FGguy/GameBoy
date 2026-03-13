#pragma once
#include "IGameBoyPlatformLayer.h"

#include <SDL.h>

class SDLGameBoyPlatformLayer : public IGameBoyPlatformLayer {
    private:
        SDL_Window* gbWindow;
        SDL_Renderer* gbRenderer;
        SDL_Texture* gbScreenTexture;
    public:
        SDLGameBoyPlatformLayer();
        ~SDLGameBoyPlatformLayer() override;
        void processInputs(InputState& state) override;
        void renderFrame(std::vector<Pixel> display_buffer) override;
};