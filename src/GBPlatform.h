#pragma once
#include <vector>

class GBPlatform {
    private:
        std::vector<std::uint8_t> bootROM;
        std::vector<std::uint8_t> cartridgeROM;

        //IO 
        std::uint8_t b_up;
        std::uint8_t b_down;
        std::uint8_t b_left;
        std::uint8_t b_right;
        std::uint8_t b_B;
        std::uint8_t b_A;
        std::uint8_t b_select;
        std::uint8_t b_start;

        //CPU
        //BUS
        //PPU

        SDL_Window* gbWindow;
        SDL_Renderer* gbRenderer;

    public:
        GBPlatform(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM);
        int BootAndExecute();
        void ProcessInputs();
};