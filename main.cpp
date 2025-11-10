#include <GBPlatform.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

/*
    Util function that loads a file into a unsigned int8 vector
*/
void LoadFileToBuffer(const std::string& filePath, std::vector<uint8_t>& buffer) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    uint8_t byte;
    size_t rIndex{0};

    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        if (rIndex >= buffer.size()) {
            throw std::runtime_error("File size exceeds buffer size");
        }
        buffer[rIndex++] = byte;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2) {
            std::cout << "No file path provided";
            return 1;
        }
        const std::string cartridgeFilepath = argv[1];

        //initialize ram and load program
        constexpr const int cartRomSize{ 0x8000 }; //TODO: Read cartridge rom header and allocate appropriate amount of memory for ROM
        std::vector<uint8_t> cartRom(cartRomSize, 0);
        LoadFileToBuffer(cartridgeFilepath, cartRom);

        //Load Boot rom
        std::filesystem::path bootPath = std::filesystem::current_path() / "dmgb_boot.bin";
        std::vector<uint8_t> bootRom(0x100, 0);
        LoadFileToBuffer(bootPath.string(), bootRom);
            
        //run program
        GBPlatform platform{bootRom, cartRom};
        return platform.bootAndExecute();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

