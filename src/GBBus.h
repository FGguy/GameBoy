#pragma once
#include <vector>

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
        void write(std::uint8_t input, std::uint16_t address);
};