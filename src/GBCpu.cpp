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
    ir_r = gbBus->read(pc_r++);

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

std::uint16_t GBCpu::getRegisterPair(RegisterPairs register_pair){
    std::uint16_t value = g_registers[register_pair*2];
    return (value << 8) | g_registers[register_pair*2 + 1];
}

void GBCpu::setRegisterPair(std::uint16_t value, RegisterPairs register_pair){
    g_registers[register_pair*2 + 1] = static_cast<std::uint8_t>(value & 0xFF);
    g_registers[register_pair*2] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
}

void GBCpu::initInstructionTables(){

//all entries in both tables need to be initialized
// B C D E H L [HL] A

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
// missing imm8 and indirect HL
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
        if (g_registers[REG_A] < g_registers[num]) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        if ((g_registers[REG_A] & 0x0F) < (g_registers[num] & 0x0F)) {
            g_registers[REG_F] |= 0b00100000;
        } else {
            g_registers[REG_F] &= 0b11011111;
        }
        g_registers[REG_A] -= g_registers[num];
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] |= 0b01000000;
    }
};
SUB_A_R(0x97, A, 1, 1);


#pragma endregion
}