#include <GBBus.h>

GBBus::GBBus(std::vector<uint8_t>& bootROM, std::vector<uint8_t>& cartridgeROM):
    bootROM{bootROM},
    cartridgeROM{cartridgeROM},
    memoryBuffer(MEMORY_SIZE, 0)
{
}

//TODO: return 0xFF on OAM blocked reads during DMA transfer
uint8_t GBBus::read(uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF && memoryBuffer[BOOTLOCK_ADDR] == BOOTLOCK_ACTIVE){
            return bootROM[address];
        }else{
            return cartridgeROM[address];
        }
    } else if (address <= MEMORY_SIZE - 1){
        return memoryBuffer[address];
    } else { // Invalid address
        return 0xFF; 
    }
}

void GBBus::write(uint8_t value, uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF && memoryBuffer[BOOTLOCK_ADDR] == BOOTLOCK_ACTIVE) return; //Ignore writes to boot ROM
        cartridgeROM[address] = value;
    } else if(0xC000 <= address && address <= 0xDFFF){
        memoryBuffer[address] = value; // work ram
        if(address <= 0xDDFF) memoryBuffer[address + 0x2000] = value; // echo ram
    } else if(0xE000 <= address && address <= 0xFDFF){
        memoryBuffer[address] = value; // echo ram
        memoryBuffer[address - 0x2000] = value; // work ram
    } else if (address <= MEMORY_SIZE - 1) {
        memoryBuffer[address] = value;
    }
}
