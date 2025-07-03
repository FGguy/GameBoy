#pragma once
#include <GBBus.h>
#include <GBCpu.h>
#include <GBPpu.h>
#include <SDL.h>

#include <vector>

class GBPlatform {
    private:
        std::vector<uint8_t> boot_ROM;
        std::vector<uint8_t> cartridge_ROM;

        //Timers
        std::uint32_t vblank_timer{0};
        std::uint32_t lcd_timer{0};
        std::uint32_t tima_timer{0};
        std::uint32_t div_timer{0};

        //IO 
        uint8_t b_up;
        uint8_t b_down;
        uint8_t b_left;
        uint8_t b_right;
        uint8_t b_B;
        uint8_t b_A;
        uint8_t b_select;
        uint8_t b_start;

        //hardware
        GBBus gbBus;
        GBCpu gbCpu;
        GBPpu gbPpu;

        SDL_Window* gbWindow;
        SDL_Renderer* gbRenderer;

        bool quit;

    public:
        GBPlatform(std::vector<uint8_t>& boot_ROM, std::vector<uint8_t>& cartridge_ROM);
        int bootAndExecute();
        void processInputs();
        void UpdateTimers(uint16_t cycles);
        void RenderFrame();
};