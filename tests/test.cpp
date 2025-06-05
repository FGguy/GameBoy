#include <catch2/catch_test_macros.hpp>
#include <GBBus.h>
#include <GBCpu.h>

TEST_CASE("Behavior of the bus", "[bus]"){
    //Roms are initialized with 0, and 1 to identify which rom is being read from.
    std::vector<std::uint8_t> cart_rom(32 * 1024 , 0);
    std::vector<std::uint8_t> boot_rom(256 , 1);
    GBBus bus{boot_rom, cart_rom};

    SECTION("Boot ROM is mapped and unmapped"){
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

    SECTION("Write to the work and echo ram are mirrored"){
        WHEN("Write are done to the work ram."){
            bus.write(0xAA, 0xC102);
            bus.write(0xAB, 0xD0F7);
            THEN("Writes are mirrored to the echo Ram"){
                //Offsetting original address for convenience to map to corresponding index in echo ram
                REQUIRE(bus.read(0xC102 + 0x2000) == 0xAA);
                REQUIRE(bus.read(0xD0F7 + 0x2000) == 0xAB);
            }
        }
    }
}

TEST_CASE("Behavior of the CPU", "[cpu]"){

    std::vector<std::uint8_t> cart_rom(32 * 1024 , 0);
    std::vector<std::uint8_t> boot_rom(256 , 1);
    GBBus bus{boot_rom, cart_rom};
    GBCpu cpu{&bus};

    SECTION("Utility methods are implemented correctly."){
        WHEN("Individual registers of a register pair are set."){
            cpu.setRegister(0xB4, REG_B);
            cpu.setRegister(0x21, REG_C);
            THEN("Their combined value reflects those changes"){
                REQUIRE(cpu.getRegisterPair(REG_BC) == 0xB421);
            }
        }

        WHEN("A register pair is set."){
            cpu.setRegisterPair(0x015F, REG_DE);
            THEN("Individual registers of that pair are set properly."){
                REQUIRE(cpu.getRegister(REG_D) == 0x01);
                REQUIRE(cpu.getRegister(REG_E) == 0x5F);
            }
        }

        WHEN("The AF register pair is set."){
            cpu.setRegisterPair(0xDEAD, REG_AF);
            THEN("The lower 4 bits of the F register are set to 0."){
                REQUIRE(cpu.getRegister(REG_F) & 0x0F == 0);
            }
        }

        WHEN("Arithmetic is performed."){
            THEN("Flags are set correctly according to the result."){
                //Addition
                cpu.setRegister(0x00, REG_H); //required, register are unitialized by default
                cpu.addToRegister(REG_H, 0x09);
                std::uint8_t flag = cpu.getRegister(REG_F);
                REQUIRE(flag & 0x10 == 0); //Carry
                REQUIRE(flag & 0x20 == 0); //Half-Carry
                REQUIRE(flag & 0x20 == 0); //Subtraction
                REQUIRE(flag & 0x80 == 0); //Zero

                cpu.setRegister(0x01, REG_H);
                cpu.addToRegister(REG_H, 0xFF);
                std::uint8_t flag = cpu.getRegister(REG_F);
                REQUIRE(flag & 0x10 == 1); //Carry
                REQUIRE(flag & 0x20 == 0); //Half-Carry
                REQUIRE(flag & 0x20 == 0); //Subtraction
                REQUIRE(flag & 0x80 == 1); //Zero

                cpu.setRegister(0x0F, REG_H);
                cpu.addToRegister(REG_H, 0x0F);
                std::uint8_t flag = cpu.getRegister(REG_F);
                REQUIRE(flag & 0x10 == 0); //Carry
                REQUIRE(flag & 0x20 == 1); //Half-Carry
                REQUIRE(flag & 0x20 == 0); //Subtraction
                REQUIRE(flag & 0x80 == 0); //Zero

                //Substraction
                cpu.setRegister(0x00, REG_H); //required, register are unitialized by default
                cpu.subFromRegister(REG_H, 0x09);
                std::uint8_t flag = cpu.getRegister(REG_F);
                REQUIRE(flag & 0x10 == 1); //Carry
                REQUIRE(flag & 0x20 == 0); //Half-Carry
                REQUIRE(flag & 0x20 == 1); //Subtraction
                REQUIRE(flag & 0x80 == 0); //Zero

                cpu.setRegister(0x0F, REG_H);
                cpu.subFromRegister(REG_H, 0x0F);
                std::uint8_t flag = cpu.getRegister(REG_F);
                REQUIRE(flag & 0x10 == 0); //Carry
                REQUIRE(flag & 0x20 == 0); //Half-Carry
                REQUIRE(flag & 0x20 == 1); //Subtraction
                REQUIRE(flag & 0x80 == 1); //Zero

                cpu.setRegister(0x18, REG_H);
                cpu.subFromRegister(REG_H, 0x0F);
                std::uint8_t flag = cpu.getRegister(REG_F);
                REQUIRE(flag & 0x10 == 0); //Carry
                REQUIRE(flag & 0x20 == 1); //Half-Carry
                REQUIRE(flag & 0x20 == 1); //Subtraction
                REQUIRE(flag & 0x80 == 0); //Zero
            }
        }
    }
}
