#include <GBBus.h>

GBBus::GBBus(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM):
    bootROM{bootROM},
    cartridgeROM{cartridgeROM},
    vRam(8192),
    extRam(8192),
    wRam(8192),
    echoRam(7680),
    OAM(160),
    IORegisters(128),
    hRam(127),
    ieRegister{0}
{
}

std::uint8_t GBBus::read(std::uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF){
            if(!IORegisters[0x50]){ //1 Boot rom is disabled, read from cartridge, 0 boot rom is active
                return bootROM[address];
            }else{
                return cartridgeROM[address];
            }
        }else{
            return cartridgeROM[address];
        }
    } else if(0x8000 <= address && address <= 0x9FFF){
        return vRam[address % 0x8000];
    } else if(0xA000 <= address && address <= 0xBFFF){
        return extRam[address % 0xA000];
    } else if(0xC000 <= address && address <= 0xDFFF){
        return wRam[address % 0xC000];
    } else if(0xE000 <= address && address <= 0xFDFF){
        return echoRam[address % 0xE000];
    } else if(0xFE00 <= address && address <= 0xFE9F){
        return OAM[address % 0xFE00];
    } else if (0xFF00 <= address && address <= 0xFF7F){ //hram access
        return IORegisters[address % 0xFF00];
    } else if (0xFF80 <= address && address <= 0xFFFE){ //hram access
        return hRam[address % 0xFF80];
    } else if (address == 0xFFFF){
        return ieRegister;
    } else {
        return 0xFF;
    }
}


//TODO: route reads to cartridge rom to MBC registers
void GBBus::write(std::uint8_t value, std::uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF){
            if(!IORegisters[0x50]){ //1 Boot rom is disabled, read from cartridge, 0 boot rom is active
                //ignore writes to boot ROM
            }else{
                cartridgeROM[address] = value;
            }
        }else{
            cartridgeROM[address] = value;
        }
    } else if(0x8000 <= address && address <= 0x9FFF){
        vRam[address % 0x8000] = value;
    } else if(0xA000 <= address && address <= 0xBFFF){
        extRam[address % 0xA000] = value;
    } else if(0xC000 <= address && address <= 0xDFFF){
        wRam[address % 0xC000] = value;
        if(address <= 0xDDFF) echoRam[address % 0xC000] = value; //emulate mirroring between wRam and echoRam
    } else if(0xE000 <= address && address <= 0xFDFF){
        echoRam[address % 0xE000] = value;
        wRam[address % 0xE000] = value; //emulate mirroring between wRam and echoRam
    } else if(0xFE00 <= address && address <= 0xFE9F){
        OAM[address % 0xFE00] = value;
    } else if (0xFF00 <= address && address <= 0xFF7F){ //hram access
        IORegisters[address % 0xFF00] = value;
    } else if (0xFF80 <= address && address <= 0xFFFE){ //hram access
        hRam[address % 0xFF80] = value;
    } else if (address == 0xFFFF){
        ieRegister = value;
    }
}