#include <catch2/catch_test_macros.hpp>
#include <GBBus.h>

TEST_CASE("Boot ROM is mapped and unmapped correctly", "[bus]"){
    //Roms are initialized with 0, and 1 to identify which rom is being read from.
    std::vector<std::uint8_t> cart_rom(32 * 1024 , 0);
    std::vector<std::uint8_t> boot_rom(256 , 1);
    GBBus bus{boot_rom, cart_rom};

    WHEN("The boot is activated."){
        bus.write(0x00, 0xFF50); //
        THEN("Reads between 0x0000-0x00FF are done to the boot rom."){
            REQUIRE(bus.read(0x004F) == 1);
            REQUIRE(bus.read(0x00FF) == 1);
            REQUIRE(bus.read(0x0100) == 0);
            REQUIRE(bus.read(0x01FF) == 0);
        }
    }

    WHEN("The boot is deactivated."){
        bus.write(0x01, 0xFF50); //
        THEN("Reads between 0x0000-0x00FF are done to the cartridge rom."){
            REQUIRE(bus.read(0x004F) == 0);
            REQUIRE(bus.read(0x00FF) == 0);
        }
    }
}