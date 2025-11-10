#include <GBBus.h>

GBBus::GBBus(std::vector<uint8_t>& bootROM, std::vector<uint8_t>& cartridgeROM):
    bootROM{bootROM},
    cartridgeROM{cartridgeROM}
{
    this->memory = vector<uint8_t>(MEMORY_SIZE, 0);
}

//TODO: return 0xFF on OAM blocked reads during DMA transfer
uint8_t GBBus::read(uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF && memory[BOOTLOCK_ADDR] == BOOTLOCK_ACTIVE){
            return bootROM[address];
        }else{
            return cartridgeROM[address];
        }
    } else if (address <= MEMORY_SIZE - 1){
        return memory[address];
    } else { // Invalid address
        return 0xFF; 
    }
}

void GBBus::write(uint8_t value, uint16_t address){
    if(address <= 0x7FFF){
        if(address <= 0x00FF && memory[BOOTLOCK_ADDR] == BOOTLOCK_ACTIVE) return; //Ignore writes to boot ROM
        cartridgeROM[address] = value;
    } else if(0xC000 <= address && address <= 0xDFFF){
        memory[address] = value; // work ram
        if(address <= 0xDDFF) memory[address + 0x2000] = value; // echo ram
    } else if(0xE000 <= address && address <= 0xFDFF){
        memory[address] = value; // echo ram
        memory[address - 0x2000] = value; // work ram
    } else if (address <= MEMORY_SIZE - 1) {
        memory[address] = value;
    }
}
