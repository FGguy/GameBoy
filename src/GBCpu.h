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
    LCD_STAT,
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
        uint16_t pc_r; //program counter
        uint16_t sp_r; //stack counter

        uint8_t ir_r; //instruction register
        uint8_t ime_r; //interrupt enable register
        std::vector<uint8_t> g_registers; //a, f, b, c, d, e, h, l

        bool g_halted;
        bool g_stopped;
        bool unimpl_instruction_reached;

        Instruction instruction_table[256];
        Instruction cb_instruction_table[256];

        uint16_t jump_vectors[5] = {0x0040, 0x0048, 0x0050, 0x0058, 0x0060};

        GBBus* gbBus;

    public:
        GBCpu(GBBus* gbBus);
        void initInstructionTables();
        uint8_t decodeExecuteInstruction();
        uint8_t handleInterrupts();
        void requestInterrupt(InterruptSource interrupt);

        //util
        uint16_t getRegisterPair(RegisterPairs register_pair);
        void setRegisterPair(uint16_t value, RegisterPairs register_pair);
        void addToRegister(Registers reg, uint8_t value);
        void subFromRegister(Registers reg, uint8_t value);
        uint8_t readR8(uint8_t r8);
        void writeR8(uint8_t value, uint8_t r8);

        //setters & getters
        uint8_t getRegister(Registers reg);
        void setRegister(uint8_t value, Registers reg);
};