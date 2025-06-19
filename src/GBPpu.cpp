#include <GBPpu.h>

/*
TODO: interrupts dispatching logic, STAT blocking
TODO: The actual graphical rendering pipeline lol
*/

GBPpu::GBPpu(GBBus* gbBus):
    gbBus{gbBus},
    sprite_buffer(10),
    ppu_timer{0},
    ppu_timer_delta{0},
    mode{PpuMode::OAM_SCAN},
    waiting{false},
    drawing_penalty{0}
{
}

void GBPpu::UpdateTimer(uint8_t cycles){
    if(gbBus->read(LCDC_ADDR) & 0b10000000){ //PPU is disabled
        waiting = false;
        ppu_timer = 0;
        ppu_timer_delta = 0;
        mode = OAM_SCAN;
        return;
    }
    ppu_timer += cycles;
    switch(mode){
        case PpuMode::OAM_SCAN:
            if(!waiting){
                //execute OAM scan
            } else if(ppu_timer >= 80){
                ChangeModes(PpuMode::DRAWING);
            }
            break;
        case PpuMode::DRAWING:
            if(!waiting){
                //execute Draw to Line
            } else if(ppu_timer >= 172 + drawing_penalty){
                ChangeModes(PpuMode::HBLANK);
            }
            break;
        case PpuMode::HBLANK:
            if(!waiting){
                //wait
            } else if(ppu_timer >= 204 - drawing_penalty){
                uint8_t ly = gbBus->read(LY_ADDR);
                if(ly < 144){
                    ChangeModes(PpuMode::OAM_SCAN);
                    gbBus->write(ly + 1, LY_ADDR);
                } else {
                    ChangeModes(PpuMode::VBLANK);
                }
            }
            break;
        case PpuMode::VBLANK:
            ppu_timer_delta += cycles;
            if(ppu_timer >= 4560){
                ChangeModes(PpuMode::OAM_SCAN);
                gbBus->write(0, LY_ADDR);
                ppu_timer_delta = 0;
            } else if(ppu_timer_delta >= 456){
                gbBus->write(gbBus->read(LY_ADDR) + 1, LY_ADDR);
                ppu_timer_delta = 0;
            }
            break;
    }
    if(gbBus->read(LY_ADDR) == gbBus->read(LYC_ADDR)){
        gbBus->write((gbBus->read(STAT_ADDR) | 0b00000100), STAT_ADDR); //LYC == LY
    }
}

void GBPpu::ChangeModes(PpuMode mode){ 
    waiting = false;
    this->mode = mode;
    ppu_timer = 0;
    gbBus->write((gbBus->read(STAT_ADDR) & 0b11111100) | static_cast<uint8_t>(mode), STAT_ADDR);
}