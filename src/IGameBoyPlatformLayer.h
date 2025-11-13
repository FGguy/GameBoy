#pragma once
#include <stdint.h>

struct InputState {
    uint8_t b_up;
    uint8_t b_down;
    uint8_t b_left;
    uint8_t b_right;
    uint8_t b_B;
    uint8_t b_A;
    uint8_t b_select;
    uint8_t b_start;
    bool quit;
    bool joypadInterruptRequested;
};

class IGameBoyPlatformLayer {
    public:
        virtual ~IGameBoyPlatformLayer() = default;
        virtual void processInputs(InputState& state) = 0;
        virtual void renderFrame() = 0; // take display buffer as parameter
};