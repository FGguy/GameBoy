#include <GBCpu.h>
#include <InstructionMacros.h>

GBCpu::GBCpu(GBBus* gbBus):
    gbBus{gbBus},
    g_registers(8),
    pc_r{0x0000},
    g_halted{false},
    g_stopped{false}, //need to turn false if true when input is detected
    unimpl_instruction_reached{false}
{
    initInstructionTables(); //causes infinite loop?
}

uint8_t GBCpu::decodeExecuteInstruction(){
    //fetch instruction opcode
    ir_r = gbBus->read(pc_r);

    //decode opcode and emulate corresponding instruction
    Instruction instruction;
    if (ir_r == 0xCB){
        ir_r = gbBus->read(pc_r + 1);
        instruction = cb_instruction_table[ir_r];
    } else {
        instruction = instruction_table[ir_r];
    }
    InstructionData data = instruction.execute();
    pc_r += data.length; //TODO: adjust accordingly to how i decide to store the instruction length in the tables.
    return data.cycles;
}

uint8_t GBCpu::handleInterrupts(){
    if(ime_r){
        uint8_t interruptQueue = gbBus->read(IF_ADDR) & gbBus->read(IE_ADDR);
        uint8_t bitmask = 0b00000001;
        for(int i = 0; i < 5; i++){
            if((bitmask & interruptQueue)){
                //push pc to stack
                ime_r = 0; //disable interrupts
                gbBus->write(gbBus->read(IF_ADDR) & ~bitmask, IF_ADDR); //flip bit of the interrupt being serviced

                //push pc to stack
                gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
                gbBus->write(static_cast<uint8_t>(pc_r), --sp_r); //lsb
                pc_r = jump_vectors[i]; //load appropriate jump vector
                return 5;
            }
            bitmask = bitmask << 1;
        }
        return 0; //no valid interrupts in queues
    } else {
        return 0;
    }
}

void GBCpu::requestInterrupt(InterruptSource interrupt){
    uint8_t bitmask = 0b00000001 << interrupt;
    gbBus->write(gbBus->read(IF_ADDR) | bitmask, IF_ADDR);
}

//For testing
uint8_t GBCpu::getRegister(Registers reg){
    return g_registers[reg];
}

void GBCpu::setRegister(uint8_t value, Registers reg){
    g_registers[reg] = value;
}

uint8_t GBCpu::readR8(uint8_t r8){
    if (r8 < 6){
        return g_registers[r8 + 2];
    } else if(r8 == 6){
        return gbBus->read(getRegisterPair(REG_HL));
    } else if(r8 == 7){
        return g_registers[REG_A];
    } else {
        return 0xFF;
    }
}

void GBCpu::writeR8(uint8_t value, uint8_t r8){
    if (r8 < 6){
        g_registers[r8 + 2] = value;
    } else if(r8 == 6){
        gbBus->write(value, getRegisterPair(REG_HL));
    } else if(r8 == 7){
        g_registers[REG_A] = value;
    }
}

void GBCpu::addToRegister(Registers reg, uint8_t value){
        if ((0xFF - g_registers[reg]) < value) { // C: Carry, overflow?
            g_registers[REG_F] |= 0b00010000; 
        } else { 
            g_registers[REG_F] &= 0b11101111; 
        } 
        if (((g_registers[reg] & 0x0F) + (value & 0x0F)) > 0x0F) { //H: Half-Carry
            g_registers[REG_F] |= 0b00100000; 
        } else { 
            g_registers[REG_F] &= 0b11011111; 
        } 
        g_registers[reg] += value;
        if (g_registers[reg] == 0){ //Z: zero, result is zero?
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; //N, substraction flag
}

void GBCpu::subFromRegister(Registers reg, uint8_t value){
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

uint16_t GBCpu::getRegisterPair(RegisterPairs register_pair){
    uint16_t value = g_registers[register_pair*2];
    return (value << 8) | g_registers[register_pair*2 + 1];
}

void GBCpu::setRegisterPair(uint16_t value, RegisterPairs register_pair){
    if (register_pair*2 + 1 == REG_F){
        g_registers[register_pair*2 + 1] = static_cast<uint8_t>(value & 0xF0);
    }else{
        g_registers[register_pair*2 + 1] = static_cast<uint8_t>(value & 0xFF);
    }
    g_registers[register_pair*2] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void GBCpu::initInstructionTables(){

const Instruction NonImplemented = {
    [this]() -> InstructionData {
        unimpl_instruction_reached = true;
        return {0, 0};
    }
};

for(int i = 0; i < 256; i++){
    instruction_table[i] = NonImplemented;
    cb_instruction_table[i] = NonImplemented;
}

#pragma region Block_1

instruction_table[0x00] = {
    [this]() -> InstructionData {
        return {1, 1};
    }
};

//ld r16, imm16
instruction_table[0x01] = {
    [this]() -> InstructionData {
        uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8);
        setRegisterPair(val, REG_BC);
        return {3, 3};
    }
};

instruction_table[0x11] = {
    [this]() -> InstructionData {
        uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8);
        setRegisterPair(val, REG_DE);
        return {3, 3};
    }
};

instruction_table[0x21] = {
    [this]() -> InstructionData {
        uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8);
        setRegisterPair(val, REG_HL);
        return {3, 3};
    }
};

instruction_table[0x31] = {
    [this]() -> InstructionData {
        uint16_t val = gbBus->read(pc_r + 1);
        val |= (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8);
        sp_r = val;
        return {3, 3};
    }
};

//ld [r16mem], a
instruction_table[0x02] = {
    [this]() -> InstructionData {
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_BC));
        return {1, 2};
    }
};

instruction_table[0x12] = {
    [this]() -> InstructionData {
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_DE));
        return {1, 2};
    }
};

instruction_table[0x22] = {
    [this]() -> InstructionData {
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) + 1, REG_HL);
        return {1, 2};
    }
};

instruction_table[0x32] = {
    [this]() -> InstructionData {
        gbBus->write(g_registers[REG_A], getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) - 1, REG_HL);
        return {1, 2};
    }
};

//ld a, [r16mem]
instruction_table[0x0A] = {
    [this]() -> InstructionData {
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_BC));
        return {1, 2};
    }
};

instruction_table[0x1A] = {
    [this]() -> InstructionData {
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_DE));
        return {1, 2};
    }
};

instruction_table[0x2A] = {
    [this]() -> InstructionData {
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) + 1, REG_HL);
        return {1, 2};
    }
};

instruction_table[0x3A] = {
    [this]() -> InstructionData {
        g_registers[REG_A] = gbBus->read(getRegisterPair(REG_HL));
        setRegisterPair(getRegisterPair(REG_HL) - 1, REG_HL);
        return {1, 2};
    }
};

//ld [imm16], sp
instruction_table[0x08] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(pc_r + 1);
        addr |= (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8);
        gbBus->write(static_cast<uint8_t>(sp_r >> 8), addr);
        return {3, 5};
    }
};

//inc r16
instruction_table[0x03] = {
    [this]() -> InstructionData {
        setRegisterPair(getRegisterPair(REG_BC) + 1, REG_BC);
        return {1, 2};
    }
};

instruction_table[0x13] = {
    [this]() -> InstructionData {
        setRegisterPair(getRegisterPair(REG_DE) + 1, REG_DE);
        return {1, 2};
    }
};

instruction_table[0x23] = {
    [this]() -> InstructionData {
        setRegisterPair(getRegisterPair(REG_HL) + 1, REG_HL);
        return {1, 2};
    }
};

instruction_table[0x33] = {
    [this]() -> InstructionData {
        sp_r += 1;
        return {1, 2};
    }
};

//dec r16
instruction_table[0x0B] = {
    [this]() -> InstructionData {
        setRegisterPair(getRegisterPair(REG_BC) - 1, REG_BC);
        return {1, 2};
    }
};

instruction_table[0x1B] = {
    [this]() -> InstructionData {
        setRegisterPair(getRegisterPair(REG_DE) - 1, REG_DE);
        return {1, 2};
    }
};

instruction_table[0x2B] = {
    [this]() -> InstructionData {
        setRegisterPair(getRegisterPair(REG_HL) - 1, REG_HL);
        return {1, 2};
    }
};

instruction_table[0x3B] = {
    [this]() -> InstructionData {
        sp_r -= 1;
        return {1, 2};
    }
};

//add hl, r16
instruction_table[0x09] = {
    [this]() -> InstructionData {
        addToRegister(REG_L, g_registers[REG_C]); //lsb
        addToRegister(REG_H, g_registers[REG_B]); //msb
        return {1, 2};
    }
};

instruction_table[0x19] = {
    [this]() -> InstructionData {
        addToRegister(REG_L, g_registers[REG_E]); //lsb
        addToRegister(REG_H, g_registers[REG_D]); //msb
        return {1, 2};
    }
};

instruction_table[0x29] = {
    [this]() -> InstructionData {
        addToRegister(REG_L, g_registers[REG_L]); //lsb
        addToRegister(REG_H, g_registers[REG_H]); //msb
        return {1, 2};
    }
};

instruction_table[0x39] = {
    [this]() -> InstructionData {
        uint8_t lsb = static_cast<uint8_t>(sp_r & 0x00FF);
        uint8_t msb = static_cast<uint8_t>(sp_r >> 8);
        addToRegister(REG_L, lsb); //lsb
        addToRegister(REG_H, msb); //msb
        return {1, 2};
    }
};

//inc r8
instruction_table[0x04] = {
    [this]() -> InstructionData {
        addToRegister(REG_B, 1);
        return {1, 1};
    }
};

instruction_table[0x0C] = {
    [this]() -> InstructionData {
        addToRegister(REG_C, 1);
        return {1, 1};
    }
};

instruction_table[0x14] = {
    [this]() -> InstructionData {
        addToRegister(REG_D, 1);
        return {1, 1};
    }
};

instruction_table[0x1C] = {
    [this]() -> InstructionData {
        addToRegister(REG_E, 1);
        return {1, 1};
    }
};

instruction_table[0x24] = {
    [this]() -> InstructionData {
        addToRegister(REG_H, 1);
        return {1, 1};
    }
};

instruction_table[0x2C] = {
    [this]() -> InstructionData {
        addToRegister(REG_L, 1);
        return {1, 1};
    }
};

instruction_table[0x34] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
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
        return {1, 3};
    }
};

instruction_table[0x3C] = {
    [this]() -> InstructionData {
        addToRegister(REG_A, 1);
        return {1, 1};
    }
};

//dec r8
instruction_table[0x05] = {
    [this]() -> InstructionData {
        subFromRegister(REG_B, 1);
        return {1, 1};
    }
};

instruction_table[0x0D] = {
    [this]() -> InstructionData {
        subFromRegister(REG_B, 1);
        return {1, 1};
    }
};

instruction_table[0x15] = {
    [this]() -> InstructionData {
        subFromRegister(REG_D, 1);
        return {1, 1};
    }
};

instruction_table[0x1D] = {
    [this]() -> InstructionData {
        subFromRegister(REG_E, 1);
        return {1, 1};
    }
};

instruction_table[0x25] = {
    [this]() -> InstructionData {
        subFromRegister(REG_H, 1);
        return {1, 1};
    }
};

instruction_table[0x2D] = {
    [this]() -> InstructionData {
        subFromRegister(REG_L, 1);
        return {1, 1};
    }
};

instruction_table[0x35] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
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
        return {1, 3};
    }
};

instruction_table[0x3D] = {
    [this]() -> InstructionData {
        subFromRegister(REG_A, 1);
        return {1, 1};
    }
};

instruction_table[0x07] = {
    [this]() -> InstructionData {
        uint8_t carry = g_registers[REG_A] >> 7;
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        g_registers[REG_A] = (g_registers[REG_A] << 1) | carry;
        return {1, 1};
    }
};

instruction_table[0x0F] = {
    [this]() -> InstructionData {
        uint8_t carry = g_registers[REG_A] << 7;
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        g_registers[REG_A] = (g_registers[REG_A] >> 1) | carry;
        return {1, 1};
    }
};

instruction_table[0x17] = {
    [this]() -> InstructionData {
        uint8_t carry = g_registers[REG_A] >> 7;
        g_registers[REG_A] = (g_registers[REG_A] << 1) | ((g_registers[REG_F] >> 4) & 0x01);
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        return {1, 1};
    }
};

instruction_table[0x1F] = {
    [this]() -> InstructionData {
        uint8_t carry = g_registers[REG_A] << 7;
        g_registers[REG_A] = (g_registers[REG_A] << 1) | ((g_registers[REG_F] << 3) & 0x80);
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        return {1, 1};
    }
};

//daa
instruction_table[0x27] = {
    [this]() -> InstructionData {
        if(g_registers[REG_F] & 0x40){ //sub
            if(g_registers[REG_F] & 0x10){ //carry
                g_registers[REG_A] -= 0x60;
            }
            if(g_registers[REG_F] & 0x20){ //half carry
                g_registers[REG_A] -= 0x06;
            }
        }else {
            if((g_registers[REG_F] & 0x10) || (g_registers[REG_A] & 0x0F) > 0x09){ //carry
                g_registers[REG_A] += 0x60;
            }
            if((g_registers[REG_F] & 0x20) || (g_registers[REG_A] & 0x0F) > 0x09){ //half carry
                g_registers[REG_A] += 0x06;
            }
        }
        return {1, 1};
    }
};

//cpl
instruction_table[0x2F] = {
    [this]() -> InstructionData {
        g_registers[REG_A] = ~g_registers[REG_A];
        g_registers[REG_F] |= 0b01100000;
        return {1, 1};
    }
};

//scf
instruction_table[0x37] = {
    [this]() -> InstructionData {
        g_registers[REG_F] |= 0b00010000;
        g_registers[REG_F] &= 0b10010000;
        return {1, 1};
    }
};

//ccf
instruction_table[0x3F] = {
    [this]() -> InstructionData {
        g_registers[REG_F] ^= 0b00010000;
        g_registers[REG_F] &= 0b10010000;
        return {1, 1};
    }
};

//jr
instruction_table[0x18] = {
    [this]() -> InstructionData {
        std::int8_t e = gbBus->read(pc_r + 1);
        pc_r = static_cast<uint16_t>((std::int32_t)pc_r + (std::int32_t)e);
        return {0, 3}; //length 2
    }
};

//jr cc
// cycles, 2 false, 3 true
instruction_table[0x20] = {
    [this]() -> InstructionData {
        if(g_registers[REG_F] & 0b00010000){
            std::int8_t e = gbBus->read(pc_r + 1);
            pc_r = static_cast<uint16_t>((std::int32_t)pc_r + (std::int32_t)e);
            return {0, 2};
        }
        return {0, 3}; //length 2
    }
};

instruction_table[0x28] = {
    [this]() -> InstructionData {
        if(!(g_registers[REG_F] & 0b00010000)){
            std::int8_t e = gbBus->read(pc_r + 1);
            pc_r = static_cast<uint16_t>((std::int32_t)pc_r + (std::int32_t)e);
            return {0, 2};
        }
        return {0, 3}; //length 2
    }
};

instruction_table[0x30] = {
    [this]() -> InstructionData {
        if(g_registers[REG_F] & 0b10000000){
            std::int8_t e = gbBus->read(pc_r + 1);
            pc_r = static_cast<uint16_t>((std::int32_t)pc_r + (std::int32_t)e);
            return {0, 2};
        }
        return {0, 3}; //length 2
    }
};

instruction_table[0x38] = {
    [this]() -> InstructionData {
        if(!(g_registers[REG_F] & 0b10000000)){
            std::int8_t e = gbBus->read(pc_r + 1);
            pc_r = static_cast<uint16_t>((std::int32_t)pc_r + (std::int32_t)e);
            return {0, 2};
        }
        return {0, 3}; //length 2
    }
};

instruction_table[0x10] = {
    [this]() -> InstructionData {
        g_stopped = true;
        return {1, 1}; //appears as two bytes but is one
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
    [this]() -> InstructionData {
        g_halted = true;
        return {1, 1};
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
    [this]() -> InstructionData {
        g_registers[REG_B] = gbBus->read(pc_r + 1);
        return {2, 2};
    }
};

instruction_table[0x0E] = {
    [this]() -> InstructionData {
        g_registers[REG_C] = gbBus->read(pc_r + 1);
        return {2, 2};
    }
};

instruction_table[0x16] = {
    [this]() -> InstructionData {
        g_registers[REG_D] = gbBus->read(pc_r + 1);
        return {2, 2};
    }
};

instruction_table[0x1E] = {
    [this]() -> InstructionData {
        g_registers[REG_E] = gbBus->read(pc_r + 1);
        return {2, 2};
    }
};

instruction_table[0x26] = {
    [this]() -> InstructionData {
        g_registers[REG_H] = gbBus->read(pc_r + 1);
        return {2, 2};
    }
};

instruction_table[0x2E] = {
    [this]() -> InstructionData {
        g_registers[REG_L] = gbBus->read(pc_r + 1);
        return {2, 2};
    }
};

instruction_table[0x36] = {
    [this]() -> InstructionData {
        gbBus->write(gbBus->read(pc_r + 1), getRegisterPair(REG_HL));
        return {2, 3};
    }
};

instruction_table[0x3E] = {
    [this]() -> InstructionData {
        g_registers[REG_A] = gbBus->read(pc_r + 1);
        return {2, 2};
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
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
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
        return {1, 2};
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
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
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
        return {1, 2};
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
    [this]() -> InstructionData {
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
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
        return {1, 2};
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
    [this]() -> InstructionData {
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
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
        return {1, 2};
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
    [this]() -> InstructionData {
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        g_registers[REG_A] &= num;
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10111111;
        g_registers[REG_F] |= 0b00100000;
        g_registers[REG_F] &= 0b11101111;
        return {1, 2};
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
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        g_registers[REG_A] ^= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
        return {1, 2};
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
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
        g_registers[REG_A] |= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
        return {1, 2};
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
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(getRegisterPair(REG_HL));
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
        return {1, 2};
    } 
};
CP_A_R(0xBF, A, 1, 1);
#pragma endregion

#pragma region Block_3

//add
instruction_table[0xC6] = { 
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(pc_r + 1);
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
        return {2, 2};
    } 
};

//adc
instruction_table[0xCE] = { 
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(pc_r + 1);
        uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
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
        return {2, 2};
    } 
};

//sub
instruction_table[0xD6] = {
    [this]() -> InstructionData {
        uint8_t num = gbBus->read(pc_r + 1);
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
        return {2, 2};
    }
};

//sbc
instruction_table[0xDE] = {
    [this]() -> InstructionData {
        uint8_t num = gbBus->read(pc_r + 1);
        uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;
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
        return {2, 2};
    }
};

//and
instruction_table[0xE6] = {
    [this]() -> InstructionData {
        uint8_t num = gbBus->read(pc_r + 1);
        g_registers[REG_A] &= num;
        if (g_registers[REG_A] == 0){
            g_registers[REG_F] |= 0b10000000;
        } else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10111111;
        g_registers[REG_F] |= 0b00100000;
        g_registers[REG_F] &= 0b11101111;
        return {2, 2};
    }
};

//xor
instruction_table[0xEE] = { 
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(pc_r + 1);
        g_registers[REG_A] ^= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
        return {2, 2};
    } 
};

//or
instruction_table[0xF6] = { 
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(pc_r + 1);
        g_registers[REG_A] |= num; 
        if (g_registers[REG_A] == 0){ 
            g_registers[REG_F] |= 0b10000000; 
        } else { 
            g_registers[REG_F] &= 0b01111111; 
        } 
        g_registers[REG_F] &= 0b10111111; 
        g_registers[REG_F] &= 0b11011111; 
        g_registers[REG_F] &= 0b11101111; 
        return {2, 2};
    } 
};

instruction_table[0xFE] = { 
    [this]() -> InstructionData { 
        uint8_t num = gbBus->read(pc_r + 1);
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
        return {2, 2};
    } 
};

//ret
instruction_table[0xC9] = { //possible bug, ambiguity in docs
    [this]() -> InstructionData { 
        uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        pc_r = addr;
        return {0, 4}; //length 1
    }
};

//reti
instruction_table[0xD9] = { //possible bug, ambiguity in docs
    [this]() -> InstructionData { 
        uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        pc_r = addr;
        ime_r = 1; //enable interrupts
        return {0, 4}; //length 1
    }
};

//ret cc
//C0, C8
//D0, D8
// 5 cycles if true, 2 cycles if false, problem with current implementation, need to change
instruction_table[0xC0] = { 
    [this]() -> InstructionData { 
        if (!(g_registers[REG_F] & 0b10000000)){
            uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
            return {0, 5}; //length 1
        }else {
            return {1, 2}; //length 1
        }
    }
};

instruction_table[0xC8] = { 
    [this]() -> InstructionData { 
        if (g_registers[REG_F] & 0b10000000){
            uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
            return {0, 5}; //length 1
        }else {
            return {1, 2}; //length 1
        }
    }
};

instruction_table[0xD0] = { 
    [this]() -> InstructionData { 
        if (!(g_registers[REG_F] & 0b00010000)){
            uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
            return {0, 5}; //length 1
        }else {
            return {1, 2}; //length 1
        }
    }
};

instruction_table[0xD8] = { 
    [this]() -> InstructionData { 
        if (g_registers[REG_F] & 0b00010000){
            uint16_t addr = gbBus->read(sp_r++); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
            pc_r = addr;
            return {0, 5}; //length 1
        }else {
            return {1, 2}; //length 1
        }
    }
};

//jmp nn, imm16
instruction_table[0xC3] = {
    [this]() -> InstructionData { 
        uint16_t addr = gbBus->read(pc_r + 1); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
        pc_r = addr;
        return {0, 4}; //length 3
    }
};

//jmp HL
instruction_table[0xE9] = {
    [this]() -> InstructionData { 
        pc_r = gbBus->read(getRegisterPair(REG_HL));
        return {0, 1}; //length 1
    }
};

//jmp cc
//true 4 cycles, false 3 cycles
//C2 ,CA, D2, DA
instruction_table[0xC2] = {
    [this]() -> InstructionData {
        if (!(g_registers[REG_F] & 0b10000000)){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
            return {0, 4}; //length 3
        }else {
            return {3, 3}; //length 3
        }
    }
};

instruction_table[0xCA] = {
    [this]() -> InstructionData {
        if (g_registers[REG_F] & 0b10000000){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
            return {0, 4}; //length 3
        }else {
            return {3, 3}; //length 3
        }
    }
};

instruction_table[0xD2] = {
    [this]() -> InstructionData {
        if (!(g_registers[REG_F] & 0b00010000)){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
            return {0, 4}; //length 3
        }else {
            return {3, 3}; //length 3
        }
    }
};

instruction_table[0xDA] = {
    [this]() -> InstructionData {
        if (g_registers[REG_F] & 0b00010000){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            pc_r = addr;
            return {0, 4}; //length 3
        }else {
            return {3, 3}; //length 3
        }
    }
};

instruction_table[0xCD] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(pc_r + 1); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
        //push current pc val to stack
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = addr;
        return {0, 6}; //length 3
    }   
};

//jump imm16 cc
//C4, CC, D4, DC
instruction_table[0xC4] = {
    [this]() -> InstructionData {
        if (!(g_registers[REG_F] & 0b10000000)){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
            return {0, 6}; //length 3
        }else {
            return {3, 3};
        }
    }   
};

instruction_table[0xCC] = {
    [this]() -> InstructionData {
        if (g_registers[REG_F] & 0b10000000){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
            return {0, 6}; //length 3
        }else {
            return {3, 3};
        }
    }   
};

instruction_table[0xD4] = {
    [this]() -> InstructionData {
        if (!(g_registers[REG_F] & 0b00010000)){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
            return {0, 6}; //length 3
        }else {
            return {3, 3};
        }
    }   
};

instruction_table[0xD4] = {
    [this]() -> InstructionData {
        if (g_registers[REG_F] & 0b00010000){
            uint16_t addr = gbBus->read(pc_r + 1); //lsb
            addr = addr | (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8); //msb
            //push current pc val to stack
            gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
            gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
            pc_r = addr;
            return {0, 6}; //length 3
        }else {
            return {3, 3};
        }
    }   
};

//RST
//C7, CF, D7, DF, E7, EF, F7, FF
instruction_table[0xC7] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0000;
        return {0, 4}; //length 1
    }   
};

instruction_table[0xCF] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0008;
        return {0, 4}; //length 1
    }   
};

instruction_table[0xD7] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0010;
        return {0, 4}; //length 1
    }   
};

instruction_table[0xDF] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0018;
        return {0, 4}; //length 1
    }   
};

instruction_table[0xE7] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0020;
        return {0, 4}; //length 1
    }   
};

instruction_table[0xEF] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0028;
        return {0, 4}; //length 1
    }   
};

instruction_table[0xF7] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0030;
        return {0, 4}; //length 1
    }   
};

instruction_table[0xFF] = {
    [this]() -> InstructionData {
        gbBus->write(static_cast<uint8_t>(pc_r >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(pc_r & 0x00FF), --sp_r); //lsb
        pc_r = 0x0038;
        return {0, 4}; //length 1
    }   
};

//push to stack
//C5, D5, E5, F5
instruction_table[0xC5] = {
    [this]() -> InstructionData {
        uint16_t addr = getRegisterPair(REG_BC);
        gbBus->write(static_cast<uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(addr & 0x00FF), --sp_r); //lsb
        return {1, 4};
    } 
};

instruction_table[0xD5] = {
    [this]() -> InstructionData {
        uint16_t addr = getRegisterPair(REG_DE);
        gbBus->write(static_cast<uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(addr & 0x00FF), --sp_r); //lsb
        return {1, 4};
    } 
};

instruction_table[0xE5] = {
    [this]() -> InstructionData {
        uint16_t addr = getRegisterPair(REG_HL);
        gbBus->write(static_cast<uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(addr & 0x00FF), --sp_r); //lsb
        return {1, 4};
    } 
};

instruction_table[0xF5] = {
    [this]() -> InstructionData {
        uint16_t addr = getRegisterPair(REG_AF);
        gbBus->write(static_cast<uint8_t>(addr >> 8), --sp_r); //msb
        gbBus->write(static_cast<uint8_t>(addr & 0x00FF), --sp_r); //lsb
        return {1, 4};
    } 
};

//pop stack to register
//C1, D1, E1, F1
instruction_table[0xC1] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_BC);
        return {1, 4};
    }
};

instruction_table[0xD1] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_DE);
        return {1, 4};
    }
};

instruction_table[0xE1] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_HL);
        return {1, 4};
    }
};

instruction_table[0xF1] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(sp_r++); //lsb
        addr = addr | (static_cast<uint16_t>(gbBus->read(sp_r++)) << 8); //msb
        setRegisterPair(addr, REG_AF);
        return {1, 4};
    }
};

//Ld
instruction_table[0xE0] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(pc_r + 1);
        addr |= 0xFF00;
        gbBus->write(g_registers[REG_A], addr);
        return {1, 4};
    }
};

instruction_table[0xE2] = {
    [this]() -> InstructionData {
        uint16_t addr = g_registers[REG_C];
        addr |= 0xFF00;
        gbBus->write(g_registers[REG_A], addr);
        return {1, 2};
    }
};

instruction_table[0xE2] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(pc_r + 1);
        addr |= (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8);
        gbBus->write(g_registers[REG_A], addr);
        return {3, 4};
    }
};

instruction_table[0xF2] = {
    [this]() -> InstructionData {
        uint16_t addr = g_registers[REG_C];
        addr |= 0xFF00;
        g_registers[REG_A] = gbBus->read(addr);
        return {1, 2};
    }
};

instruction_table[0xF0] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(pc_r + 1);
        addr |= 0xFF00;
        g_registers[REG_A] = gbBus->read(addr);
        return {2, 3};
    }
};

instruction_table[0xFA] = {
    [this]() -> InstructionData {
        uint16_t addr = gbBus->read(pc_r + 1);
        addr |= (static_cast<uint16_t>(gbBus->read(pc_r + 2)) << 8);
        g_registers[REG_A] = gbBus->read(addr);
        return {3, 4};
    }
};

//possible bug
instruction_table[0xE8] = {
    [this]() -> InstructionData {
        uint8_t e = gbBus->read(pc_r + 1);
        if ((0xFF - e) < (sp_r & 0x00FF)) {
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
        return {2, 4};
    }
};

instruction_table[0xF8] = {
    [this]() -> InstructionData {
        uint8_t e = gbBus->read(pc_r + 1);
        if ((0xFF - e) < (sp_r & 0x00FF)) {
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
        return {2, 3};
    }
};

instruction_table[0xF9] = {
    [this]() -> InstructionData {
        sp_r = getRegisterPair(REG_HL);
        return {1, 2};
    }
};

instruction_table[0xF3] = {
    [this]() -> InstructionData {
        ime_r = 0;
        return {1, 1};
    }
};

instruction_table[0xFB] = {
    [this]() -> InstructionData {
        ime_r = 1;
        return {1, 1};
    }
};
#pragma endregion

#pragma region CB_prefixed


RLC_R8(0x00, B, 2, 2);
RLC_R8(0x01, C, 2, 2);
RLC_R8(0x02, D, 2, 2);
RLC_R8(0x03, E, 2, 2);
RLC_R8(0x04, H, 2, 2);
RLC_R8(0x05, L, 2, 2);
cb_instruction_table[0x06] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = operand >> 7;
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        operand = (operand << 1) | carry;
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 4};
    }
};
RLC_R8(0x07, A, 2, 2);

RRC_R8(0x08, B, 2, 2);
RRC_R8(0x09, C, 2, 2);
RRC_R8(0x0A, D, 2, 2);
RRC_R8(0x0B, E, 2, 2);
RRC_R8(0x0C, H, 2, 2);
RRC_R8(0x0D, L, 2, 2);
cb_instruction_table[0x0E] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = operand << 7;
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        operand = (operand >> 1) | carry;
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 4};
    }
};
RRC_R8(0x0F, A, 2, 2);

RL_R8(0x10, B, 2, 2);
RL_R8(0x11, C, 2, 2);
RL_R8(0x12, D, 2, 2);
RL_R8(0x13, E, 2, 2);
RL_R8(0x14, H, 2, 2);
RL_R8(0x15, L, 2, 2);
cb_instruction_table[0x16] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = operand >> 7;
        operand = (operand << 1) | ((g_registers[REG_F] >> 4) & 0x01);
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 4};
    }
};
RL_R8(0x17, A, 2, 2);

RR_R8(0x18, B, 2, 2);
RR_R8(0x19, C, 2, 2);
RR_R8(0x1A, D, 2, 2);
RR_R8(0x1B, E, 2, 2);
RR_R8(0x1C, H, 2, 2);
RR_R8(0x1D, L, 2, 2);
cb_instruction_table[0x1E] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = operand << 7;
        operand = (operand >> 1) | ((g_registers[REG_F] << 3) & 0x80);
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 4};
    }
};
RR_R8(0x1F, A, 2, 2);

SLA_R8(0x20, B, 2, 2);
SLA_R8(0x21, C, 1, 2);
SLA_R8(0x22, D, 2, 2);
SLA_R8(0x23, E, 2, 2);
SLA_R8(0x24, H, 2, 2);
SLA_R8(0x25, L, 2, 2);
cb_instruction_table[0x26] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = operand >> 7;
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        operand = (operand << 1);
        if(!operand){
            g_registers[REG_F] |= 0b10000000;
        }else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10011111;
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 4};
    }
};
SLA_R8(0x27, A, 2, 2);

SRA_R8(0x28, B, 2, 2);
SRA_R8(0x29, C, 1, 2);
SRA_R8(0x2A, D, 2, 2);
SRA_R8(0x2B, E, 2, 2);
SRA_R8(0x2C, H, 2, 2);
SRA_R8(0x2D, L, 2, 2);
cb_instruction_table[0x2E] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = operand << 7;
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        operand = (operand >> 1);
        if(!operand){
            g_registers[REG_F] |= 0b10000000;
        }else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10011111;
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 4};
    }
};
SRA_R8(0x2F, A, 2, 2);

SWAP_R8(0x30, B, 2, 2);
SWAP_R8(0x31, C, 2, 2);
SWAP_R8(0x32, D, 2, 2);
SWAP_R8(0x33, E, 2, 2);
SWAP_R8(0x34, H, 2, 2);
SWAP_R8(0x35, L, 2, 2);
cb_instruction_table[0x36] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t higher_n = operand << 4;
        uint8_t lower_n = operand >> 4;
        operand = higher_n | lower_n;
        if(!operand){
            g_registers[REG_F] |= 0b10000000;
        }else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10001111;
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 3};
    }
};
SWAP_R8(0x37, A, 2, 2);

SRL_R8(0x38, B, 2, 2);
SRL_R8(0x39, C, 2, 2);
SRL_R8(0x3A, D, 2, 2);
SRL_R8(0x3B, E, 2, 2);
SRL_R8(0x3C, H, 2, 2);
SRL_R8(0x3D, L, 2, 2);
cb_instruction_table[0x3E] = {
    [this]() -> InstructionData {
        uint8_t operand = gbBus->read(getRegisterPair(REG_HL));
        uint8_t carry = operand << 7;
        if (carry) {
            g_registers[REG_F] |= 0b00010000;
        } else {
            g_registers[REG_F] &= 0b11101111;
        }
        operand = (operand >> 1);
        if(!operand){
            g_registers[REG_F] |= 0b10000000;
        }else {
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10011111;
        gbBus->write(operand, getRegisterPair(REG_HL));
        return {2, 4};
    }
};
SRL_R8(0x3F, A, 2, 2);


//Too lazy to do all of these individually, also kinda useless to do so.
Instruction bit_r8 = {
    [this]() -> InstructionData {
        uint8_t opcode = gbBus->read(pc_r + 1);
        uint8_t r8 = opcode & 0b00000111;        
        uint8_t b3 = (opcode >> 3) & 0b00000111;
        uint8_t operand = readR8(r8);
        operand = operand >> b3;
        if (operand & 0x01){
            g_registers[REG_F] |= 0b10000000;
        }else{
            g_registers[REG_F] &= 0b01111111;
        }
        g_registers[REG_F] &= 0b10111111;
        g_registers[REG_F] |= 0b00100000;
        if(r8 == 6){
            return {2, 3}; //HL
        } else {
            return {2, 2};
        }
    }   
};

for(uint16_t i = 0x40; i < 0x80; i++){
    cb_instruction_table[i] = bit_r8;
}



Instruction res_r8 = {
    [this]() -> InstructionData {
        uint8_t opcode = gbBus->read(pc_r + 1);
        uint8_t r8 = opcode & 0b00000111;        
        uint8_t b3 = (opcode >> 3) & 0b00000111;
        uint8_t operand = readR8(r8);
        uint8_t bitmask = 0b00000001 << b3;
        writeR8(operand | ~bitmask, r8);
        if(r8 == 6){
            return {2, 4}; //HL
        } else {
            return {2, 2};
        }
    }   
};

for(uint16_t i = 0x80; i < 0xC0; i++){
    cb_instruction_table[i] = res_r8;
}



Instruction set_r8 = {
    [this]() -> InstructionData {
        uint8_t opcode = gbBus->read(pc_r + 1);
        uint8_t r8 = opcode & 0b00000111;        
        uint8_t b3 = (opcode >> 3) & 0b00000111;
        uint8_t operand = readR8(r8);
        uint8_t bit = 0b00000001 << b3;
        writeR8(operand | bit, r8);
        if(r8 == 6){
            return {2, 4}; //HL
        } else {
            return {2, 2};
        }
    }   
};

for(uint16_t i = 0xC0; i <= 0xFF; i++){
    cb_instruction_table[i] = set_r8;
}

#pragma endregion

}