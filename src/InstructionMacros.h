#pragma once

#define LD_REG_REG(opcode, dst, src, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ g_registers[REG_##dst] = g_registers[REG_##src]; } \
    };

#define LD_REG_MEM_HL(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ g_registers[REG_##dst] = gbBus->read(getRegisterPair(REG_HL)); } \
    };

#define LD_REG_HL_MEM(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ gbBus->write(g_registers[REG_##dst], getRegisterPair(REG_HL)); } \
    };