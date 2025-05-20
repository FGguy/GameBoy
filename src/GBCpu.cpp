#include <GBCpu.h>

GBCpu::GBCpu(GBBus* gbBus):
    gbBus{gbBus},
    pc_r{0x0000}
{
}

std::uint8_t GBCpu::decodeExecuteInstruction(){
    //fetch instruction opcode
    ir_r = gbBus->read(pc_r++);

    //decode opcode and emulate corresponding instruction
}

std::uint16_t GBCpu::getRegisterPair(RegisterPairs register_pair){
    std::uint16_t value = g_registers[register_pair*2];
    return (value << 8) | g_registers[register_pair*2 + 1];
}

void GBCpu::setRegisterPair(std::uint16_t value, RegisterPairs register_pair){
    g_registers[register_pair*2 + 1] = static_cast<std::uint8_t>(value & 0xFF);
    g_registers[register_pair*2] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
};