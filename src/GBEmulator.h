#pragma once
#include <GBBus.h>
#include <GBCpu.h>
#include <GBPpu.h>
#include <IGameBoyPlatformLayer.h>

#include <vector>
using std::vector;

class GBEmulator {
    private:
        vector<uint8_t> bootROM;
        vector<uint8_t> cartridgeROM;

        //Timers
        uint32_t vblankTimer{0};
        uint32_t lcdTimer{0};
        uint32_t timaTimer{0};
        uint32_t divTimer{0};

        //IO 
        InputState inputState{0,0,0,0,0,0,0,0,false,false};
        void mapInputsToMemory();

        //hardware
        GBBus gbBus;
        GBCpu gbCpu;
        GBPpu gbPpu;
        IGameBoyPlatformLayer* platformLayer;

    public:
        GBEmulator(vector<uint8_t>& bootROM, vector<uint8_t>& cartridgeROM);
        ~GBEmulator();
        int bootAndExecute();
        void updateTimers(uint16_t cycles);
};