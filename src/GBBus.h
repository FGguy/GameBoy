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
#define DMA_ADDR 0xFF46 //writing to this starts obj_attribute_mem DMA, lasts 160 M-cycles, bus is blocked for cpu, ppu cant read obj_attribute_mem
#define BGP_ADDR 0xFF47
#define OBP0_ADDR 0xFF48
#define OBP1_ADDR 0xFF49

class GBBus{
    private:
        std::vector<uint8_t> boot_ROM; //0000 - 00FF
        std::vector<uint8_t> cartridge_ROM; //0000 - 3FFF fixed, 4000 - 7FFF switchable. Will probably need to create cartridge class to hide mapper from bus

        std::vector<uint8_t> v_ram; //8000 - 9FFF
        std::vector<uint8_t> ext_ram; //A000 - BFFF
        std::vector<uint8_t> w_ram; //C000 - DFFF
        std::vector<uint8_t> echo_ram; //E000 - FDFF
        std::vector<uint8_t> obj_attribute_mem; //FE00 - FE9F
        std::vector<uint8_t> h_ram; //FE00 - FE9F
        std::vector<uint8_t> io_registers; //FF00 - FF7F

        uint8_t ie_register; //FFFF

    public:
        GBBus(std::vector<uint8_t>& boot_ROM, std::vector<uint8_t>& cartridge_ROM);
        uint8_t read(uint16_t address);
        void write(uint8_t value, uint16_t address);
};