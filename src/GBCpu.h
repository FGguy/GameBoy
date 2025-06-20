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

enum InterruptSource {
    VBlank,
    LCD,
    Timer,
    Serial,
    Joypad
};

struct InstructionData {
    uint8_t length;
    uint8_t cycles;
};

struct Instruction {
    std::function<InstructionData()> execute;
};

class GBCpu {
    private:
        std::uint16_t pc_r; //program counter
        std::uint16_t sp_r; //stack counter

        std::uint8_t ir_r; //instruction register
        std::uint8_t ime_r; //interrupt enable register
        std::vector<std::uint8_t> g_registers; //a, f, b, c, d, e, h, l

        bool g_halted;
        bool g_stopped;
        bool unimpl_instruction_reached;

        Instruction instruction_table[256];
        Instruction cb_instruction_table[256];

        std::uint16_t jump_vectors[5] = {0x0040, 0x0048, 0x0050, 0x0058, 0x0060};

        GBBus* gbBus;

    public:
        GBCpu(GBBus* gbBus);
        void initInstructionTables();
        std::uint8_t decodeExecuteInstruction();
        std::uint8_t handleInterrupts();
        void requestInterrupt(InterruptSource interrupt);

        //util
        std::uint16_t getRegisterPair(RegisterPairs register_pair);
        void setRegisterPair(std::uint16_t value, RegisterPairs register_pair);
        void addToRegister(Registers reg, std::uint8_t value);
        void subFromRegister(Registers reg, std::uint8_t value);
        std::uint8_t readR8(std::uint8_t r8);
        void writeR8(std::uint8_t value, std::uint8_t r8);

        //setters & getters
        std::uint8_t getRegister(Registers reg);
        void setRegister(std::uint8_t value, Registers reg);
};