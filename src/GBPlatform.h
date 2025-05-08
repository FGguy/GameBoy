#pragma once
#include <vector>

class GBPlatform 
{
    private:
        std::vector<std::uint8_t> bootROM;
        std::vector<std::uint8_t> cartridgeROM;

        //CPU
        //BUS
        //PPU

        SDL_Window* gbWindow;
        SDL_Renderer* gbRenderer;

    public:
        GBPlatform(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM);
        int BootAndExecute();
};