#pragma once
#include <vector>
using std::vector;

constexpr const uint32_t MEMORY_SIZE = 0x10000; //64KB

constexpr const uint16_t BOOTLOCK_DISABLED = 1;
constexpr const uint16_t BOOTLOCK_ACTIVE = 0;

constexpr const uint16_t IE_ADDR = 0xFFFF;
constexpr const uint16_t IF_ADDR = 0xFF0F;
constexpr const uint16_t BOOTLOCK_ADDR = 0xFF50;
constexpr const uint16_t DIV_ADDR = 0xFF04;
constexpr const uint16_t TIMA_ADDR = 0xFF05;
constexpr const uint16_t TMA_ADDR = 0xFF06;
constexpr const uint16_t TAC_ADDR = 0xFF07;
constexpr const uint16_t JOYP_ADDR = 0xFF00;

//graphics
constexpr const uint16_t LCDC_ADDR = 0xFF40;
constexpr const uint16_t STAT_ADDR = 0xFF41;
constexpr const uint16_t SCY_ADDR = 0xFF42;
constexpr const uint16_t SCX_ADDR = 0xFF43;
constexpr const uint16_t LY_ADDR = 0xFF44;
constexpr const uint16_t LYC_ADDR = 0xFF45;
constexpr const uint16_t WY_ADDR = 0xFF4A;
constexpr const uint16_t WX_ADDR = 0xFF4B;

// writing to this starts obj_attribute_mem DMA, lasts 160 M-cycles, bus is blocked for cpu, ppu cant read obj_attribute_mem
constexpr const uint16_t DMA_ADDR = 0xFF46; 

constexpr const uint16_t BGP_ADDR = 0xFF47;
constexpr const uint16_t OBP0_ADDR = 0xFF48;
constexpr const uint16_t OBP1_ADDR = 0xFF49;

/*
Mappings:
0000 - 00FF   Boot ROM (mapped only if BOOTLOCK is active)
0000 - 7FFF   Cartridge ROM
8000 - 9FFF   Video RAM (VRAM)
A000 - BFFF   External RAM (cartridge RAM)
C000 - DFFF   Work RAM (WRAM)
E000 - FDFF   Echo RAM (mirrors C000 - DDFF)
FE00 - FE9F   Sprite Attribute Table (OAM)
FEA0 - FEFF   Not Usable
FF00 - FF7F   I/O Registers
FF80 - FFFE   High RAM (HRAM)
FFFF         Interrupt Enable Register (IE)
*/

class GBBus{
    private:
        /*
        0000 - 3FFF fixed, 4000 - 7FFF switchable
        TODO: create cartridge class to hide mapper from bus
        */
        vector<uint8_t> cartridgeROM; 
        vector<uint8_t> bootROM;
        vector<uint8_t> memoryBuffer;

    public:
        GBBus(vector<uint8_t>& bootROM, vector<uint8_t>& cartridgeROM);
        uint8_t read(uint16_t address);
        void write(uint8_t value, uint16_t address);
};