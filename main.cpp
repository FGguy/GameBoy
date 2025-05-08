#include <GBPlatform.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <chrono>

/*
    Util function that loads a file into a unsigned int8 vector
*/
enum Status {
    OK,
    ERR_OPEN,
    ERR_OVERFLOW
};

Status LoadFileToBuffer(const std::string& filePath, std::vector<std::uint8_t>& buffer) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return Status::ERR_OPEN;
    }

    std::uint8_t byte;
    size_t r_index{0};

    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        if (r_index >= buffer.size()) {
            return Status::ERR_OVERFLOW;
        }
        buffer[r_index++] = byte;
    }

    return Status::OK;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << "No file path provided";
        return 1;
    }
    const std::string cartridge_filepath = argv[1];

    //initialize ram and load program
    int kb{ 1024 };
    int cart_rom_size{ 32*kb }; //TODO: Read cartridge rom header and allocate appropriate amount of memory for ROM
    std::vector<std::uint8_t> cart_rom(cart_rom_size,0);

    switch (LoadFileToBuffer(cartridge_filepath, cart_rom))
    {
    case ERR_OPEN:
        std::cerr << "Error: Failed to open the cartridge rom file." << cartridge_filepath << std::endl;
        return 1;
        break;
    case ERR_OVERFLOW:
        std::cerr << "Error: cartridge rom file is too big for memory." << std::endl;
        return 1;
        break;
    default:
        break;
    }

    //Load Boot rom
    std::vector<std::uint8_t> boot_rom(256,0);
    switch (LoadFileToBuffer("C:/Users/Yan/Desktop/gameboy/dmg_boot.bin", boot_rom)) //TODO: Change to use a relative path
    {
    case ERR_OPEN:
        std::cerr << "Error: Could not load the boot rom, make sure it is in the same directory as the executable." << std::endl;
        return 1;
        break;
    case ERR_OVERFLOW:
        std::cerr << "Error: boot rom file is too big for memory." << std::endl;
        return 1;
        break;
    default:
        break;
    }

    //run program
    GBPlatform platform{boot_rom, cart_rom};
    return platform.BootAndExecute();
}

