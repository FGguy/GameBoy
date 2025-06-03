#include <GBCpu.h>
#include <InstructionMacros.h>

GBCpu::GBCpu(GBBus* gbBus):
    gbBus{gbBus},
    pc_r{0x0000},
    g_halted{false}
{
    initInstructionTables();
}

std::uint8_t GBCpu::decodeExecuteInstruction(){
    //fetch instruction opcode
    ir_r = gbBus->read(pc_r);

    //decode opcode and emulate corresponding instruction
    Instruction instruction;
    if (ir_r == 0xCB){
        ir_r = gbBus->read(pc_r);
        instruction = cb_instruction_table[ir_r];
    } else {
        instruction = instruction_table[ir_r];
    }
    instruction.execute();
    pc_r += instruction.length; //TODO: adjust accordingly to how i decide to store the instruction length in the tables.
    return instruction.cycles;
}

void GBCpu::addToRegister(Registers reg, std::uint8_t value){
        if ((0xFF - g_registers[reg]) < value) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[reg] & 0x0F) + (value & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[reg] += value;
        if (g_registers[reg] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
}

void GBCpu::subFromRegister(Registers reg, std::uint8_t value){
    if (g_registers[reg] < value) {
        g_registers[REG_F] |= 0b00010000;
    } else {
        g_registers[REG_F] &= 0b11101111;
    }
    if ((g_registers[reg] & 0x0F) < (value & 0x0F)) {
        g_registers[REG_F] |= 0b00100000;
    } else {
        g_registers[REG_F] &= 0b11011111;
    }
    g_registers[reg] -= value;
    if (g_registers[reg] == 0){
        g_registers[REG_F] |= 0b10000000;
    } else {
        g_registers[REG_F] &= 0b01111111;
    }
    g_registers[REG_F] |= 0b01000000;
}

std::uint16_t GBCpu::getRegisterPair(RegisterPairs register_pair){
    std::uint16_t value = g_registers[register_pair*2];
    return (value << 8) | g_registers[register_pair*2 + 1];
}

void GBCpu::setRegisterPair(std::uint16_t value, RegisterPairs register_pair){
    g_registers[register_pair*2 + 1] = static_cast<std::uint8_t>(value & 0xFF);
    g_registers[register_pair*2] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
}

void GBCpu::initInstructionTables(){
//need to refactor instructions to return cycles and length
//potential bug, incrementing the pc after setting its value;
//all entries in both tables need to be initialized;
//why are flags set on increments and decrements on individual registers but not register pairs
// B C D E H L [HL] A

#pragma region Block_1

instruction_table[0x00] = {
    1,
    1,
    [&](){

    }
};

instruction_table[0x01] = {
    3,
    3,
    [&](){
        std::uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8);
        setRegisterPair(val, REG_BC);
    }
};

instruction_table[0x11] = {
    3,
    3,
    [&](){
        std::uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8);
        setRegisterPair(val, REG_DE);
    }
};

instruction_table[0x21] = {
    3,
    3,
    [&](){
        std::uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8);
        setRegisterPair(val, REG_HL);
    }
};

instruction_table[0x31] = {
    3,
    3,
    [&](){
        std::uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8);
        setRegisterPair(val, REG_AF);
    }
};

instruction_table[0x02] = {
    1,
    2,
    [&](){
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_HL));
    }
};

instruction_table[0x12] = {
    1,
    2,
    [&](){
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_DE));
    }
};

instruction_table[0x22] = {
    1,
    2,
    [&](){
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) + 1, REG_HL);
    }
};

instruction_table[0x32] = {
    1,
    2,
    [&](){
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) - 1, REG_HL);
    }
};

instruction_table[0x0A] = {
    1,
    2,
    [&](){
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_BC));
    }
};

instruction_table[0x1A] = {
    1,
    2,
    [&](){
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_DE));
    }
};

instruction_table[0x2A] = {
    1,
    2,
    [&](){
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) + 1, REG_HL);
    }
};

instruction_table[0x3A] = {
    1,
    2,
    [&](){
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) - 1, REG_HL);
    }
};

instruction_table[0x08] = {
    3,
    5,
    [&](){
        std::uint16_t addr = gbBus->read(pc_r + 1);
        addr |= (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8);
        gbBus->write(static_cast<std::uint8_t>(sp_r >> 8), addr);
    }
};

instruction_table[0x03] = {
    1,
    2,
    [&](){
        setRegisterPair(getRegisterPair(REG_BC) + 1, REG_BC);
    }
};

instruction_table[0x13] = {
    1,
    2,
    [&](){
        setRegisterPair(getRegisterPair(REG_DE) + 1, REG_DE);
    }
};

instruction_table[0x23] = {
    1,
    2,
    [&](){
        setRegisterPair(getRegisterPair(REG_HL) + 1, REG_HL);
    }
};

instruction_table[0x33] = {
    1,
    2,
    [&](){
        sp_r += 1;
    }
};

instruction_table[0x0B] = {
    1,
    2,
    [&](){
        setRegisterPair(getRegisterPair(REG_BC) - 1, REG_BC);
    }
};

instruction_table[0x1B] = {
    1,
    2,
    [&](){
        setRegisterPair(getRegisterPair(REG_DE) - 1, REG_DE);
    }
};

instruction_table[0x2B] = {
    1,
    2,
    [&](){
        setRegisterPair(getRegisterPair(REG_HL) - 1, REG_HL);
    }
};

instruction_table[0x3B] = {
    1,
    2,
    [&](){
        sp_r -= 1;
    }
};

instruction_table[0x09] = {
    1,
    2,
    [&](){
        //lsb
        if ((0xFF - g_registers[REG_L]) < g_registers[REG_C]) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_L] & 0x0F) + (g_registers[REG_C] & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_L] += g_registers[REG_C];
        if (g_registers[REG_L] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 

        //msb
        if ((0xFF - g_registers[REG_H]) < g_registers[REG_B]) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_H] & 0x0F) + (g_registers[REG_B] & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_H] += g_registers[REG_B];
        if (g_registers[REG_H] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    }
};

instruction_table[0x19] = {
    1,
    2,
    [&](){
        if ((0xFF - g_registers[REG_L]) < g_registers[REG_E]) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_L] & 0x0F) + (g_registers[REG_E] & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_L] += g_registers[REG_E];
        if (g_registers[REG_L] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 

        //msb
        if ((0xFF - g_registers[REG_H]) < g_registers[REG_D]) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_H] & 0x0F) + (g_registers[REG_D] & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_H] += g_registers[REG_D];
        if (g_registers[REG_H] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    }
};

instruction_table[0x29] = {
    1,
    2,
    [&](){
        if ((0xFF - g_registers[REG_L]) < g_registers[REG_L]) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_L] & 0x0F) + (g_registers[REG_L] & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_L] += g_registers[REG_L];
        if (g_registers[REG_L] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 

        //msb
        if ((0xFF - g_registers[REG_H]) < g_registers[REG_H]) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_H] & 0x0F) + (g_registers[REG_H] & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_H] += g_registers[REG_H];
        if (g_registers[REG_H] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    }
};

instruction_table[0x39] = {
    1,
    2,
    [&](){
        std::uint8_t lsb = static_cast<std::uint8_t>(sp_r & 0x00FF);
        std::uint8_t msb = static_cast<std::uint8_t>(sp_r >> 8);
        if ((0xFF - g_registers[REG_L]) < lsb) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_L] & 0x0F) + (lsb & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_L] += lsb;
        if (g_registers[REG_L] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 

        //msb
        if ((0xFF - g_registers[REG_H]) < msb) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_H] & 0x0F) + (msb & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_H] += msb;
        if (g_registers[REG_H] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    }
};

instruction_table[0x04] = {
    1,
    1,
    [&](){
        addToRegister(REG_B, 1);
    }
};

instruction_table[0x0C] = {
    1,
    1,
    [&](){
        addToRegister(REG_C, 1);
    }
};

instruction_table[0x14] = {
    1,
    1,
    [&](){
        addToRegister(REG_D, 1);
    }
};

instruction_table[0x1C] = {
    1,
    1,
    [&](){
        addToRegister(REG_E, 1);
    }
};

instruction_table[0x24] = {
    1,
    1,
    [&](){
        addToRegister(REG_H, 1);
    }
};

instruction_table[0x2C] = {
    1,
    1,
    [&](){
        addToRegister(REG_L, 1);
    }
};

instruction_table[0x34] = {
    1,
    3,
    [&](){
        std::uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        if ((0xFF - operand) < 1) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((operand & 0x0F) + (1 & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        operand += 1;
        if (operand == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        gbBus->write(operand, getRegisterPair(REG_HL));
    }
};

instruction_table[0x3C] = {
    1,
    1,
    [&](){
        addToRegister(REG_A, 1);
    }
};

//dec r8
instruction_table[0x05] = {
    1,
    1,
    [&](){
        subFromRegister(REG_B, 1);
    }
};

instruction_table[0x0D] = {
    1,
    1,
    [&](){
        subFromRegister(REG_B, 1);
    }
};

instruction_table[0x15] = {
    1,
    1,
    [&](){
        subFromRegister(REG_D, 1);
    }
};

instruction_table[0x1D] = {
    1,
    1,
    [&](){
        subFromRegister(REG_E, 1);
    }
};

instruction_table[0x25] = {
    1,
    1,
    [&](){
        subFromRegister(REG_H, 1);
    }
};

instruction_table[0x2D] = {
    1,
    1,
    [&](){
        subFromRegister(REG_L, 1);
    }
};

instruction_table[0x35] = {
    1,
    1,
    [&](){
        std::uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        if (operand < 1) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if ((operand & 0x0F) < (1 & 0x0F)) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        operand -= 1;
        if (operand == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] |= 0b01000000;
        gbBus->write(operand, getRegisterPair(REG_HL));
    }
};

instruction_table[0x3D] = {
    1,
    1,
    [&](){
        subFromRegister(REG_A, 1);
    }
};

instruction_table[0x07] = {
    1,
    1,
    [&](){
        subFromRegister(REG_A, 1);
    }
};
#pragma endregion

//LD r, r' Load Instructions
#pragma region LD_R_R
LD_REG_REG(0x40, B, B, 1, 1);
LD_REG_REG(0x41, B, C, 1, 1);
LD_REG_REG(0x42, B, D, 1, 1);
LD_REG_REG(0x43, B, E, 1, 1);
LD_REG_REG(0x44, B, H, 1, 1);
LD_REG_REG(0x45, B, L, 1, 1);
LD_REG_MEM_HL(0x46, B, 1, 2);
LD_REG_REG(0x47, B, A, 1, 1);

LD_REG_REG(0x48, C, B, 1, 1);
LD_REG_REG(0x49, C, C, 1, 1);
LD_REG_REG(0x4A, C, D, 1, 1);
LD_REG_REG(0x4B, C, E, 1, 1);
LD_REG_REG(0x4C, C, H, 1, 1);
LD_REG_REG(0x4D, C, L, 1, 1);
LD_REG_MEM_HL(0x4E, C, 1, 2);
LD_REG_REG(0x4F, C, A, 1, 1);

LD_REG_REG(0x50, D, B, 1, 1);
LD_REG_REG(0x51, D, C, 1, 1);
LD_REG_REG(0x52, D, D, 1, 1);
LD_REG_REG(0x53, D, E, 1, 1);
LD_REG_REG(0x54, D, H, 1, 1);
LD_REG_REG(0x55, D, L, 1, 1);
LD_REG_MEM_HL(0x56, D, 1, 2);
LD_REG_REG(0x57, D, A, 1, 1);

LD_REG_REG(0x58, E, B, 1, 1);
LD_REG_REG(0x59, E, C, 1, 1);
LD_REG_REG(0x5A, E, D, 1, 1);
LD_REG_REG(0x5B, E, E, 1, 1);
LD_REG_REG(0x5C, E, H, 1, 1);
LD_REG_REG(0x5D, E, L, 1, 1);
LD_REG_MEM_HL(0x5E, E, 1, 2);
LD_REG_REG(0x5F, E, A, 1, 1);

LD_REG_REG(0x60, H, B, 1, 1);
LD_REG_REG(0x61, H, C, 1, 1);
LD_REG_REG(0x62, H, D, 1, 1);
LD_REG_REG(0x63, H, E, 1, 1);
LD_REG_REG(0x64, H, H, 1, 1);
LD_REG_REG(0x65, H, L, 1, 1);
LD_REG_MEM_HL(0x66, H, 1, 2);
LD_REG_REG(0x67, H, A, 1, 1);

LD_REG_REG(0x68, L, B, 1, 1);
LD_REG_REG(0x69, L, C, 1, 1);
LD_REG_REG(0x6A, L, D, 1, 1);
LD_REG_REG(0x6B, L, E, 1, 1);
LD_REG_REG(0x6C, L, H, 1, 1);
LD_REG_REG(0x6D, L, L, 1, 1);
LD_REG_MEM_HL(0x6E, L, 1, 2);
LD_REG_REG(0x6F, L, A, 1, 1);

LD_REG_HL_MEM(0x70, B, 1, 2);
LD_REG_HL_MEM(0x71, C, 1, 2);
LD_REG_HL_MEM(0x72, D, 1, 2);
LD_REG_HL_MEM(0x73, E, 1, 2);
LD_REG_HL_MEM(0x74, H, 1, 2);
LD_REG_HL_MEM(0x75, L, 1, 2);
instruction_table[0x76] = {
    1,
    1,
    [&](){
        g_halted = true;
    }
};

LD_REG_HL_MEM(0x77, A, 1, 8);

LD_REG_REG(0x78, A, B, 1, 1);
LD_REG_REG(0x79, A, C, 1, 1);
LD_REG_REG(0x7A, A, D, 1, 1);
LD_REG_REG(0x7B, A, E, 1, 1);
LD_REG_REG(0x7C, A, H, 1, 1);
LD_REG_REG(0x7D, A, L, 1, 1);
LD_REG_MEM_HL(0x7E, A, 1, 2);
LD_REG_REG(0x7F, A, A, 1, 1);
#pragma endregion

//LD r, imm8 Load immediate next byte into register
#pragma region LD_R_IMM
instruction_table[0x06] = {
    2,
    2,
    [&](){
        g_registers[REG_B] = gbBus->read(pc_r + 1);
    }
};

instruction_table[0x0E] = {
    2,
    2,
    [&](){
        g_registers[REG_C] = gbBus->read(pc_r + 1);
    }
};

instruction_table[0x16] = {
    2,
    2,
    [&](){
        g_registers[REG_D] = gbBus->read(pc_r + 1);
    }
};

instruction_table[0x1E] = {
    2,
    2,
    [&](){
        g_registers[REG_E] = gbBus->read(pc_r + 1);
    }
};

instruction_table[0x26] = {
    2,
    2,
    [&](){
        g_registers[REG_H] = gbBus->read(pc_r + 1);
    }
};

instruction_table[0x2E] = {
    2,
    2,
    [&](){
        g_registers[REG_L] = gbBus->read(pc_r + 1);
    }
};

instruction_table[0x36] = {
    2,
    2,
    [&](){
        gbBus->write(gbBus->read(pc_r + 1), getRegisterPair(REG_HL));
    }
};

instruction_table[0x3E] = {
    2,
    2,
    [&](){
        g_registers[REG_A] = gbBus->read(pc_r + 1);
    }
};
#pragma endregion

// Operation between A and R
// missing imm8
#pragma region Block_2
    // 0x80 - 0xBF
ADD_A_R(0x80, B, 1, 1);
ADD_A_R(0x81, C, 1, 1);
ADD_A_R(0x82, D, 1, 1);
ADD_A_R(0x83, E, 1, 1);
ADD_A_R(0x84, H, 1, 1);
ADD_A_R(0x85, L, 1, 1);
instruction_table[0x86] = { 
    1, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        if ((0xFF - g_registers[REG_A]) < num) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_A] & 0x0F) + (num & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_A] += num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    } 
};
ADD_A_R(0x87, A, 1, 1);

ADC_A_R(0x88, B, 1, 1);
ADC_A_R(0x89, C, 1, 1);
ADC_A_R(0x8A, D, 1, 1);
ADC_A_R(0x8B, E, 1, 1);
ADC_A_R(0x8C, H, 1, 1);
ADC_A_R(0x8D, L, 1, 1);
instruction_table[0x8E] = { 
    1, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        std::uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
        if ((0xFF - g_registers[REG_A]) < (num + carry)) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_A] & 0x0F) + (num & 0x0F) + carry ) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_A] += num + carry; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    } 
};
ADC_A_R(0x8F, A, 1, 1);

SUB_A_R(0x90, B, 1, 1);
SUB_A_R(0x91, C, 1, 1);
SUB_A_R(0x92, D, 1, 1);
SUB_A_R(0x93, E, 1, 1);
SUB_A_R(0x94, H, 1, 1);
SUB_A_R(0x95, L, 1, 1);
instruction_table[0x96] = {
    1,
    2,
    [&](){
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        if (g_registers[REG_A] < num) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if ((g_registers[REG_A] & 0x0F) < (num & 0x0F)) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        g_registers[REG_A] -= num;
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] |= 0b01000000;
    }
};
SUB_A_R(0x97, A, 1, 1);

SBC_A_R(0x98, B, 1, 1);
SBC_A_R(0x99, C, 1, 1);
SBC_A_R(0x9A, D, 1, 1);
SBC_A_R(0x9B, E, 1, 1);
SBC_A_R(0x9C, H, 1, 1);
SBC_A_R(0x9D, L, 1, 1);
instruction_table[0x9E] = {
    1,
    2,
    [&](){
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        std::uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
        if (g_registers[REG_A] < (num + carry)) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if ((g_registers[REG_A] & 0x0F) < (num & 0x0F + carry)) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        g_registers[REG_A] -= (num + carry);
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] |= 0b01000000;
    }
};
SBC_A_R(0x9F, A, 1, 1);

AND_A_R(0xA0, B, 1, 1);
AND_A_R(0xA1, C, 1, 1);
AND_A_R(0xA2, D, 1, 1);
AND_A_R(0xA3, E, 1, 1);
AND_A_R(0xA4, H, 1, 1);
AND_A_R(0xA5, L, 1, 1);
instruction_table[0xA6] = {
    1,
    2,
    [&](){
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        g_registers[REG_A] &= num;
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10111111;
        g_registers[REG_F] |= 0b00100000;
        g_registers[REG_F] &= 0b11101111;
    }
};
AND_A_R(0xA7, A, 1, 1);

XOR_A_R(0xA8, B, 1, 1);
XOR_A_R(0xA9, C, 1, 1);
XOR_A_R(0xAA, D, 1, 1);
XOR_A_R(0xAB, E, 1, 1);
XOR_A_R(0xAC, H, 1, 1);
XOR_A_R(0xAD, L, 1, 1);
instruction_table[0xAE] = { 
    1, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        g_registers[REG_A] ^= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
    } 
};
XOR_A_R(0xAF, A, 1, 1);

OR_A_R(0xB0, B, 1, 1);
OR_A_R(0xB1, C, 1, 1);
OR_A_R(0xB2, D, 1, 1);
OR_A_R(0xB3, E, 1, 1);
OR_A_R(0xB4, H, 1, 1);
OR_A_R(0xB5, L, 1, 1);
instruction_table[0xB6] = { 
    1, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        g_registers[REG_A] |= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
    } 
};
OR_A_R(0xB7, A, 1, 1);

CP_A_R(0xB8, B, 1, 1);
CP_A_R(0xB9, C, 1, 1);
CP_A_R(0xBA, D, 1, 1);
CP_A_R(0xBB, E, 1, 1);
CP_A_R(0xBC, H, 1, 1);
CP_A_R(0xBD, L, 1, 1);
instruction_table[0xBE] = { 
    1, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        if (g_registers[REG_A] < num) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if ((g_registers[REG_A] & 0x0F) < (num & 0x0F)) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] |= 0b01000000; 
    } 
};
CP_A_R(0xBF, A, 1, 1);
#pragma endregion

#pragma region Block_3

//add
instruction_table[0xC6] = { 
    2, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(pc_r + 1);
        if ((0xFF - g_registers[REG_A]) < num) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_A] & 0x0F) + (num & 0x0F)) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_A] += num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    } 
};

//adc
instruction_table[0xCE] = { 
    2, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(pc_r + 1);
        std::uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
        if ((0xFF - g_registers[REG_A]) < (num + carry)) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[REG_A] & 0x0F) + (num & 0x0F) + carry ) > 0x0F) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[REG_A] += num + carry; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
    } 
};

//sub
instruction_table[0xD6] = {
    2,
    2,
    [&](){
        std::uint8_t num = gbBus->read(pc_r + 1);
        if (g_registers[REG_A] < num) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if ((g_registers[REG_A] & 0x0F) < (num & 0x0F)) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        g_registers[REG_A] -= num;
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] |= 0b01000000;
    }
};

//sbc
instruction_table[0xDE] = {
    2,
    2,
    [&](){
        std::uint8_t num = gbBus->read(pc_r + 1);
        std::uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
        if (g_registers[REG_A] < (num + carry)) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if ((g_registers[REG_A] & 0x0F) < (num & 0x0F + carry)) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        g_registers[REG_A] -= (num + carry);
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] |= 0b01000000;
    }
};

//and
instruction_table[0xE6] = {
    2,
    2,
    [&](){
        std::uint8_t num = gbBus->read(pc_r + 1);
        g_registers[REG_A] &= num;
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10111111;
        g_registers[REG_F] |= 0b00100000;
        g_registers[REG_F] &= 0b11101111;
    }
};

//xor
instruction_table[0xEE] = { 
    2, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(pc_r + 1);
        g_registers[REG_A] ^= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
    } 
};

//or
instruction_table[0xF6] = { 
    2, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(pc_r + 1);
        g_registers[REG_A] |= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
    } 
};

instruction_table[0xFE] = { 
    2, 
    2, 
    [&](){ 
        std::uint8_t num = gbBus->read(pc_r + 1);
        if (g_registers[REG_A] < num) { 
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if ((g_registers[REG_A] & 0x0F) < (num & 0x0F)) { 
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] |= 0b01000000; 
    } 
};

//ret
instruction_table[0xC9] = { //possible bug, ambiguity in docs
    1, 
    4, 
    [&](){ 
        std::uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        pc_r = addr;
    }
};

//reti
instruction_table[0xD9] = { //possible bug, ambiguity in docs
    1, 
    4, 
    [&](){ 
        std::uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        pc_r = addr;
        ime_r = 1; //enable interrupts
    }
};

//ret cc
//C0, C8
//D0, D8
// 5 cycles if true, 2 cycles if false, problem with current implementation, need to change
instruction_table[0xC0] = { 
    1, 
    4, 
    [&](){ 
        if (!(g_registers[REG_F] & 0b10000000)){
            std::uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
        }
    }
};
instruction_table[0xC8] = { 
    1, 
    4, 
    [&](){ 
        if (g_registers[REG_F] & 0b10000000){
            std::uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
        }
    }
};
instruction_table[0xD0] = { 
    1, 
    4, 
    [&](){ 
        if (!(g_registers[REG_F] & 0b00010000)){
            std::uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
        }
    }
};
instruction_table[0xD8] = { 
    1, 
    4, 
    [&](){ 
        if (g_registers[REG_F] & 0b00010000){
            std::uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
        }
    }
};

//jmp nn, imm16
instruction_table[0xC3] = {
    3,
    4,
    [&](){ 
        std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
        pc_r = addr;
    }
};

//jmp HL
instruction_table[0xE9] = {
    1,
    1,
    [&](){ 
        pc_r = gbBus->read(getRegisterPair(REG_HL));
    }
};

//jmp cc
//true 4 cycles, false 3 cycles
//C2 ,CA, D2, DA
instruction_table[0xC2] = {
    3,
    4,
    [&](){
        if (!(g_registers[REG_F] & 0b10000000)){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
        }
    }
};

instruction_table[0xCA] = {
    3,
    4,
    [&](){
        if (g_registers[REG_F] & 0b10000000){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
        }
    }
};

instruction_table[0xD2] = {
    3,
    4,
    [&](){
        if (!(g_registers[REG_F] & 0b00010000)){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
        }
    }
};

instruction_table[0xDA] = {
    3,
    4,
    [&](){
        if (g_registers[REG_F] & 0b00010000){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
        }
    }
};

instruction_table[0xCD] = {
    3,
    6,
    [&](){
        std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
        //push current pc val to stack
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = addr;
    }   
};

//jump imm16 cc
//C4, CC, D4, DC
instruction_table[0xC4] = {
    3,
    6,
    [&](){
        if (!(g_registers[REG_F] & 0b10000000)){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
        }
    }   
};

instruction_table[0xCC] = {
    3,
    6,
    [&](){
        if (g_registers[REG_F] & 0b10000000){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
        }
    }   
};

instruction_table[0xD4] = {
    3,
    6,
    [&](){
        if (!(g_registers[REG_F] & 0b00010000)){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
        }
    }   
};

instruction_table[0xD4] = {
    3,
    6,
    [&](){
        if (g_registers[REG_F] & 0b00010000){
            std::uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
        }
    }   
};

//RST
//C7, CF, D7, DF, E7, EF, F7, FF
instruction_table[0xC7] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0000;
    }   
};

instruction_table[0xCF] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0008;
    }   
};

instruction_table[0xD7] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0010;
    }   
};

instruction_table[0xDF] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0018;
    }   
};

instruction_table[0xE7] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0020;
    }   
};

instruction_table[0xEF] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0028;
    }   
};

instruction_table[0xF7] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0030;
    }   
};

instruction_table[0xFF] = {
    1,
    4,
    [&](){
        gbBus->write(static_cast<std::uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0038;
    }   
};

//push to stack
//C5, D5, E5, F5
instruction_table[0xC5] = {
    1,
    4,
    [&](){
        std::uint16_t addr = getRegisterPair(REG_BC);
        gbBus->write(static_cast<std::uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(addr & 0x00FF), --sp_r); //lsb
    } 
};

instruction_table[0xD5] = {
    1,
    4,
    [&](){
        std::uint16_t addr = getRegisterPair(REG_DE);
        gbBus->write(static_cast<std::uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(addr & 0x00FF), --sp_r); //lsb
    } 
};

instruction_table[0xE5] = {
    1,
    4,
    [&](){
        std::uint16_t addr = getRegisterPair(REG_HL);
        gbBus->write(static_cast<std::uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(addr & 0x00FF), --sp_r); //lsb
    } 
};

instruction_table[0xF5] = {
    1,
    4,
    [&](){
        std::uint16_t addr = getRegisterPair(REG_AF);
        gbBus->write(static_cast<std::uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<std::uint8_t>(addr & 0x00FF), --sp_r); //lsb
    } 
};

//pop stack to register
//C1, D1, E1, F1
instruction_table[0xC1] = {
    1,
    4,
    [&](){
        std::uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_BC);
    }
};

instruction_table[0xD1] = {
    1,
    4,
    [&](){
        std::uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_DE);
    }
};

instruction_table[0xE1] = {
    1,
    4,
    [&](){
        std::uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_HL);
    }
};

instruction_table[0xF1] = {
    1,
    4,
    [&](){
        std::uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<std::uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_AF);
    }
};

//Ld
instruction_table[0xE0] = {
    2,
    3,
    [&](){
        std::uint16_t addr = gbBus->read(pc_r + 1);
        addr |= 0xFF00;
        gbBus->write(g_registers[REG_A], addr);
    }
};

instruction_table[0xE2] = {
    1,
    2,
    [&](){
        std::uint16_t addr = g_registers[REG_C];
        addr |= 0xFF00;
        gbBus->write(g_registers[REG_A], addr);
    }
};

instruction_table[0xE2] = {
    3,
    4,
    [&](){
        std::uint16_t addr = gbBus->read(pc_r + 1);
        addr |= (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8);
        gbBus->write(g_registers[REG_A], addr);
    }
};

instruction_table[0xF2] = {
    1,
    2,
    [&](){
        std::uint16_t addr = g_registers[REG_C];
        addr |= 0xFF00;
        g_registers[REG_A] = gbBus->read(addr);
    }
};

instruction_table[0xF0] = {
    2,
    3,
    [&](){
        std::uint16_t addr = gbBus->read(pc_r + 1);
        addr |= 0xFF00;
        g_registers[REG_A] = gbBus->read(addr);
    }
};

instruction_table[0xFA] = {
    3,
    4,
    [&](){
        std::uint16_t addr = gbBus->read(pc_r + 1);
        addr |= (static_cast<std::uint16_t>(gbBus->read(pc_r + 2)) << 8);
        g_registers[REG_A] = gbBus->read(addr);
    }
};

//possible bug
instruction_table[0xE8] = {
    2,
    4,
    [&](){
        std::uint8_t e = gbBus->read(pc_r + 1);
        if ((0xFF - e) < sp_r & 0x00FF) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if (((e & 0x0F) + (sp_r & 0x0F)) > 0x0F) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        sp_r += e;
        g_registers[REG_F] &= 0b01111111;
        g_registers[REG_F] &= 0b10111111;
    }
};

instruction_table[0xF8] = {
    2,
    3,
    [&](){
        std::uint8_t e = gbBus->read(pc_r + 1);
        if ((0xFF - e) < sp_r & 0x00FF) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if (((e & 0x0F) + (sp_r & 0x0F)) > 0x0F) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        setRegisterPair(sp_r + e, REG_HL);
        g_registers[REG_F] &= 0b01111111;
        g_registers[REG_F] &= 0b10111111;
    }
};

instruction_table[0xF9] = {
    1,
    2,
    [&](){
        sp_r = getRegisterPair(REG_HL);
    }
};

instruction_table[0xF3] = {
    1,
    1,
    [&](){
        ime_r = 0;
    }
};

instruction_table[0xFB] = {
    1,
    1,
    [&](){
        ime_r = 1;
    }
};
#pragma endregion
}