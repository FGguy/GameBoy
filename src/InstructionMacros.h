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

#define ADD_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            if ((0xFF - g_registers[REG_A]) < g_registers[REG_##dst]) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            if (((g_registers[REG_A] & 0x0F) + (g_registers[REG_##dst] & 0x0F)) > 0x0F) { \
                g_registers[REG_F] |= 0b00100000; \
            } else { \
                g_registers[REG_F] &= 0b11011111; \
            } \
            g_registers[REG_A] += g_registers[REG_##dst]; \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10111111; \
        } \
    };

#define ADC_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;\
            if ((0xFF - g_registers[REG_A]) < (g_registers[REG_##dst] + carry)) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            if (((g_registers[REG_A] & 0x0F) + (g_registers[REG_##dst] & 0x0F) + carry ) > 0x0F) { \
                g_registers[REG_F] |= 0b00100000; \
            } else { \
                g_registers[REG_F] &= 0b11011111; \
            } \
            g_registers[REG_A] += g_registers[REG_##dst] + carry; \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10111111; \
        } \
    };

#define SUB_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            if (g_registers[REG_A] < g_registers[REG_##dst]) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            if ((g_registers[REG_A] & 0x0F) < (g_registers[REG_##dst] & 0x0F)) { \
                g_registers[REG_F] |= 0b00100000; \
            } else { \
                g_registers[REG_F] &= 0b11011111; \
            } \
            g_registers[REG_A] -= g_registers[REG_##dst]; \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] |= 0b01000000; \
        } \
    };

#define SBC_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = (g_registers[REG_F] & 0b00010000) >> 4;\
            if (g_registers[REG_A] < (g_registers[REG_##dst] + carry)) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            if ((g_registers[REG_A] & 0x0F) < (g_registers[REG_##dst] & 0x0F + carry)) { \
                g_registers[REG_F] |= 0b00100000; \
            } else { \
                g_registers[REG_F] &= 0b11011111; \
            } \
            g_registers[REG_A] -= (g_registers[REG_##dst] + carry); \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] |= 0b01000000; \
        } \
    };

#define CP_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            if (g_registers[REG_A] < g_registers[REG_##dst]) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            if ((g_registers[REG_A] & 0x0F) < (g_registers[REG_##dst] & 0x0F)) { \
                g_registers[REG_F] |= 0b00100000; \
            } else { \
                g_registers[REG_F] &= 0b11011111; \
            } \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] |= 0b01000000; \
        } \
    };

#define AND_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            g_registers[REG_A] &= g_registers[REG_##dst]; \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10111111; \
            g_registers[REG_F] |= 0b00100000; \
            g_registers[REG_F] &= 0b11101111; \
        } \
    };

#define XOR_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            g_registers[REG_A] ^= g_registers[REG_##dst]; \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10111111; \
            g_registers[REG_F] &= 0b11011111; \
            g_registers[REG_F] &= 0b11101111; \
        } \
    };

#define OR_A_R(opcode, dst, length, cycles) \
    instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            g_registers[REG_A] |= g_registers[REG_##dst]; \
            if (g_registers[REG_A] == 0){ \
                g_registers[REG_F] |= 0b10000000; \
            } else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10111111; \
            g_registers[REG_F] &= 0b11011111; \
            g_registers[REG_F] &= 0b11101111; \
        } \
    };


//CB PREFIXED

//RLC
#define RLC_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = g_registers[REG_##dst] >> 7; \
            if (carry) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            g_registers[REG_##dst] = (g_registers[REG_##dst] << 1) | carry; \
        } \
    };


//RRC
#define RRC_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = g_registers[REG_##dst] << 7; \
            if (carry) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            g_registers[REG_##dst] = (g_registers[REG_##dst] >> 1) | carry; \
        } \
    };

//RL
#define RL_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = g_registers[REG_##dst] >> 7; \
            g_registers[REG_##dst] = (g_registers[REG_##dst] << 1) | ((g_registers[REG_F] >> 4) & 0x01); \
            if (carry) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
        } \
    };


//RR
#define RR_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = g_registers[REG_##dst] << 7; \
            g_registers[REG_##dst] = (g_registers[REG_##dst] >> 1) | ((g_registers[REG_F] << 3) & 0x80); \
            if (carry) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
        } \
    };

#define SLA_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = g_registers[REG_##dst] >> 7; \
            if (carry) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            g_registers[REG_##dst] = (g_registers[REG_##dst] << 1); \
            if(!g_registers[REG_##dst]){ \
                g_registers[REG_F] |= 0b10000000; \
            }else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10011111; \
        } \
    };

#define SRA_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = g_registers[REG_##dst] << 7; \
            if (carry) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            g_registers[REG_##dst] = (g_registers[REG_##dst] >> 1) | carry; \
            if(!g_registers[REG_##dst]){ \
                g_registers[REG_F] |= 0b10000000; \
            }else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10011111; \
        } \
    };

#define SWAP_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t higher_n = g_registers[REG_##dst] << 4; \
            std::uint8_t lower_n = g_registers[REG_##dst] >> 4; \
            g_registers[REG_##dst] = higher_n | lower_n; \
            if(!g_registers[REG_##dst]){ \
                g_registers[REG_F] |= 0b10000000; \
            }else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10001111; \
        } \
    };

#define SRL_R8(opcode, dst, length, cycles) \
    cb_instruction_table[opcode] = { \
        length, \
        cycles, \
        [&](){ \
            std::uint8_t carry = g_registers[REG_##dst] << 7; \
            if (carry) { \
                g_registers[REG_F] |= 0b00010000; \
            } else { \
                g_registers[REG_F] &= 0b11101111; \
            } \
            g_registers[REG_##dst] = (g_registers[REG_##dst] >> 1); \
            if(!g_registers[REG_##dst]){ \
                g_registers[REG_F] |= 0b10000000; \
            }else { \
                g_registers[REG_F] &= 0b01111111; \
            } \
            g_registers[REG_F] &= 0b10011111; \
        } \
    };

