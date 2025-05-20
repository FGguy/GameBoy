#pragma once
#include <GBBus.h>

#include <vector>
#include <functional>

enum RegisterPairs {
    REG_AF,
    REG_BC,
    REG_DE,
    REG_HL
};

enum Registers {
    REG_A,
    REG_F,
    REG_B,
    REG_C,
    REG_D,
    REG_E,
    REG_H,
    REG_L
};

struct Instruction {
    uint8_t length;
    uint8_t cycles;
    std::function<void()> execute;
};

class GBCpu {
    private:
        std::uint16_t pc_r; //program counter
        std::uint16_t sp_r; //stack counter

        std::uint8_t ir_r; //instruction register
        std::uint8_t ime_r; //interrupt enable register
        std::vector<std::uint8_t> g_registers; //a, f, b, c, d, e, h, l

        bool g_halted;

        Instruction instruction_table[256];
        Instruction cb_instruction_table[256];

        GBBus* gbBus;

    public:
        GBCpu(GBBus* gbBus);
        void initInstructionTables();
        std::uint8_t decodeExecuteInstruction();
        std::uint8_t handleInterrupts();
        std::uint16_t getRegisterPair(RegisterPairs register_pair);
        void setRegisterPair(std::uint16_t value, RegisterPairs register_pair);
};