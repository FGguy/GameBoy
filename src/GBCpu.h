#pragma once
#include <GBBus.h>

#include <vector>

class GBCpu {
    private:
        std::uint16_t pc_r; //program counter
        std::uint16_t sp_r; //stack counter

        std::uint8_t ir_r; //instruction register
        std::uint8_t ie_r; //interrupt enable register
        std::uint8_t a_r; //accumulator register
        std::uint8_t f_r; //flag register

        //General purpose register
        std::uint8_t b_r;
        std::uint8_t c_r;
        std::uint8_t d_r;
        std::uint8_t e_r;
        std::uint8_t h_r;
        std::uint8_t l_r;

        GBBus* gbBus;
    public:
        GBCpu(GBBus* gbBus);
        void executionLoop();
        void decodeExecuteInstruction();
        void fetchInstruction();
        void clockCycle();
};