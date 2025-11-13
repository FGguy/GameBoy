#include <GBEmulator.h>
#include <SDLGameBoyPlatformLayer.h>

GBEmulator::GBEmulator(vector<uint8_t>& bootROM, vector<uint8_t>& cartridgeROM):
    bootROM{bootROM},
    cartridgeROM{cartridgeROM},
    gbBus{bootROM, cartridgeROM},
    gbCpu{&gbBus},
    gbPpu{&gbBus}
{
    platformLayer = new SDLGameBoyPlatformLayer();
}

int GBEmulator::bootAndExecute(){
    gbBus.write(0x00, BOOTLOCK_ADDR); //enabled boot rom

    /*
        VBLANK 59.7 times/s DONE
        STAT/LCD 
        TIMER everytime the timer overflows DONE
        Serial will not be implemented TECHNICALLY DONE
        JOYPAD anytime one of the bits in P1 changes DONE
    */
    while(!inputState.quit){
        uint16_t cycles{0};
        cycles += gbCpu.decodeExecuteInstruction();
        cycles += gbCpu.handleInterrupts();

        gbPpu.updateTimer(cycles);
        updateTimers(cycles);

        this->platformLayer->processInputs(inputState);
        if(inputState.joypadInterruptRequested){
            inputState.joypadInterruptRequested = false;
            gbCpu.requestInterrupt(Joypad);
        }
        mapInputsToMemory();
    }

    return 0;
}

//uses M-cycles 1.048576 MHz
inline void GBEmulator::updateTimers(uint16_t cycles){
    vblankTimer += cycles; //59.7 times a sec, every 17564 M-Cycles
    if (vblankTimer >= 17564){
        gbCpu.requestInterrupt(VBlank);
        vblankTimer = 0;
        platformLayer->renderFrame();
    }
    lcdTimer += cycles;
    
    uint8_t tac = gbBus.read(TAC_ADDR);
    if(tac & 0b00000100){
        timaTimer += cycles; //incremented based on tac reg 00: 4096M, 01: 262144M, 10: 65536M 11: 16384, bit 2 is checked to see if timer is enabled, is reset to TMA value
        uint8_t tma = gbBus.read(TMA_ADDR);
        switch (tac & 0b00000011)
        {
        case 0b00000000:
            if (timaTimer >= 4096){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                timaTimer = 0;
            }
            break;

        case 0b00000001:
            if (timaTimer >= 262144){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                timaTimer = 0;
            }
            break;

        case 0b00000010:
            if (timaTimer >= 65536){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                timaTimer = 0;
            }
            break;

        case 0b00000011:
            if (timaTimer >= 16384){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                timaTimer = 0;
            }
            break;
        }
        if (gbBus.read(TIMA_ADDR) == 0){
            gbCpu.requestInterrupt(Timer);
        }
    }
    divTimer += cycles; //16384Hz, writing to it resets to 0, resets during stop, is not incremente during stop.
    if (divTimer >= 64){
        gbBus.write(gbBus.read(DIV_ADDR) + 1, DIV_ADDR);
    }

    gbBus.write(gbBus.read(DIV_ADDR) + 1,DIV_ADDR);
}

void GBEmulator::mapInputsToMemory(){
    if((~gbBus.read(JOYP_ADDR) & 0b00010000)){ //d-pad
        gbBus.write((0b00100000 | inputState.b_down << 3 | inputState.b_up << 2 | inputState.b_left << 1 | inputState.b_right), JOYP_ADDR);
    } else if((~gbBus.read(JOYP_ADDR) & 0b00100000)){ // select buttons
        gbBus.write((0b00010000 | inputState.b_start << 3 | inputState.b_select << 2 | inputState.b_B << 1 | inputState.b_A), JOYP_ADDR);
    }
}

GBEmulator::~GBEmulator(){
    delete platformLayer;
}