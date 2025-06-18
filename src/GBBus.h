#pragma once
#include <vector>

#define IE_ADDR 0xFFFF
#define IF_ADDR 0xFF0F
#define BOOTLOCK_ADDR 0xFF50
#define DIV_ADDR 0xFF04
#define TIMA_ADDR 0xFF05
#define TMA_ADDR 0xFF06
#define TAC_ADDR 0xFF07
#define JOYP_ADDR 0xFF00

//graphics
#define LCDC_ADDR 0xFF40
#define STAT_ADDR 0xFF41
#define SCY_ADDR 0xFF42
#define SCX_ADDR 0xFF43
#define LY_ADDR 0xFF44
#define LYC_ADDR 0xFF45
#define WY_ADDR 0xFF4A
#define WX_ADDR 0xFF4B
#define DMA_ADDR 0xFF46 //writing to this starts OAM DMA, lasts 160 M-cycles, bus is blocked for cpu, ppu cant read OAM
#define BGP_ADDR 0xFF47
#define OBP0_ADDR 0xFF48
#define OBP1_ADDR 0xFF49


/*
TODO:
    - Reads to IO registers
    - Timers and other special register between FF00 - FF7F
*/

class GBBus{
    private:
        std::vector<std::uint8_t> bootROM; //0000 - 00FF
        std::vector<std::uint8_t> cartridgeROM; //0000 - 3FFF fixed, 4000 - 7FFF switchable. Will probably need to create cartridge class to hide mapper from bus

        std::vector<std::uint8_t> vRam; //8000 - 9FFF
        std::vector<std::uint8_t> extRam; //A000 - BFFF
        std::vector<std::uint8_t> wRam; //C000 - DFFF
        std::vector<std::uint8_t> echoRam; //E000 - FDFF
        std::vector<std::uint8_t> OAM; //FE00 - FE9F
        std::vector<std::uint8_t> hRam; //FE00 - FE9F
        std::vector<std::uint8_t> IORegisters; //FF00 - FF7F

        std::uint8_t ieRegister; //FFFF

    public:
        GBBus(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM);
        std::uint8_t read(std::uint16_t address);
        void write(std::uint8_t value, std::uint16_t address);
};