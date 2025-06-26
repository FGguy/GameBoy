#pragma once
#include <GBBus.h>
#include <GBCpu.h>
#include <GBPpu.h>
#include <SDL.h>

#include <vector>

class GBPlatform {
    private:
        std::vector<std::uint8_t> bootROM;
        std::vector<std::uint8_t> cartridgeROM;

        //Timers
        std::uint32_t vblank_timer{0};
        std::uint32_t lcd_timer{0};
        std::uint32_t tima_timer{0};
        std::uint32_t div_timer{0};

        //IO 
        std::uint8_t b_up;
        std::uint8_t b_down;
        std::uint8_t b_left;
        std::uint8_t b_right;
        std::uint8_t b_B;
        std::uint8_t b_A;
        std::uint8_t b_select;
        std::uint8_t b_start;

        //hardware
        GBBus gbBus;
        GBCpu gbCpu;
        GBPpu gbPpu;

        SDL_Window* gbWindow;
        SDL_Renderer* gbRenderer;

        bool quit;

    public:
        GBPlatform(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM);
        int BootAndExecute();
        void ProcessInputs();
        void UpdateTimers(std::uint16_t cycles);
        void RenderFrame();
};