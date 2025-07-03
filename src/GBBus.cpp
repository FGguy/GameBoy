#include <GBBus.h>

GBBus::GBBus(std::vector<uint8_t>& boot_ROM, std::vector<uint8_t>& cartridge_ROM):
    boot_ROM{boot_ROM},
    cartridge_ROM{cartridge_ROM},
    v_ram(8192),
    ext_ram(8192),
    w_ram(8192),
    echo_ram(7680),
    obj_attribute_mem(160),
    io_registers(128),
    h_ram(127),
    ie_register{0}
{
}

uint8_t GBBus::read(uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF){
            if(io_registers[0x50] == 0){ //1 Boot rom is disabled, read from cartridge, 0 boot rom is active
                return boot_ROM[address];
            }else{
                return cartridge_ROM[address];
            }
        }else{
            return cartridge_ROM[address];
        }
    } else if(0x8000 <= address && address <= 0x9FFF){
        return v_ram[address % 0x8000];
    } else if(0xA000 <= address && address <= 0xBFFF){
        return ext_ram[address % 0xA000];
    } else if(0xC000 <= address && address <= 0xDFFF){
        return w_ram[address % 0xC000];
    } else if(0xE000 <= address && address <= 0xFDFF){
        return echo_ram[address % 0xE000];
    } else if(0xFE00 <= address && address <= 0xFE9F){
        return obj_attribute_mem[address % 0xFE00];
    } else if (0xFF00 <= address && address <= 0xFF7F){ //h_ram access
        return io_registers[address % 0xFF00];
    } else if (0xFF80 <= address && address <= 0xFFFE){ //h_ram access
        return h_ram[address % 0xFF80];
    } else if (address == 0xFFFF){
        return ie_register;
    } else {
        return 0xFF;
    }
}

//TODO: route reads to cartridge rom to MBC registers
void GBBus::write(uint8_t value, uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF){
            if(io_registers[0x50] == 0){ //1 Boot rom is disabled, read from cartridge, 0 boot rom is active
                //ignore writes to boot ROM
            }else{
                cartridge_ROM[address] = value;
            }
        }else{
            cartridge_ROM[address] = value;
        }
    } else if(0x8000 <= address && address <= 0x9FFF){
        v_ram[address % 0x8000] = value;
    } else if(0xA000 <= address && address <= 0xBFFF){
        ext_ram[address % 0xA000] = value;
    } else if(0xC000 <= address && address <= 0xDFFF){
        w_ram[address % 0xC000] = value;
        if(address <= 0xDDFF) echo_ram[address % 0xC000] = value; //emulate mirroring between w_ram and echo_ram
    } else if(0xE000 <= address && address <= 0xFDFF){
        echo_ram[address % 0xE000] = value;
        w_ram[address % 0xE000] = value; //emulate mirroring between w_ram and echo_ram
    } else if(0xFE00 <= address && address <= 0xFE9F){
        obj_attribute_mem[address % 0xFE00] = value;
    } else if (0xFF00 <= address && address <= 0xFF7F){ //h_ram access
        io_registers[address % 0xFF00] = value;
    } else if (0xFF80 <= address && address <= 0xFFFE){ //h_ram access
        h_ram[address % 0xFF80] = value;
    } else if (address == 0xFFFF){
        ie_register = value;
    }
}