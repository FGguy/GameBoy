#include <GBPpu.h>

/*
TODO: The actual graphical rendering pipeline lol

bits 6-3 are used to enable interrupts caused by a transition to a certain mode
STAT interrupt only caused by rising edge on interrupt line
what causes the interrupt line to reset, when are the bits set low?
No interrupt is dispatched while a condition is still being met

mode change? Lyc == Ly

if yes check condition bit 

if yes raise interrupt

set stat_blocking to true

stat_blocking to set to false by a check that evaluates to false


*/

GBPpu::GBPpu(GBBus* gbBus):
    gbBus{gbBus},
    sprite_buffer(10),
    ppu_timer{0},
    ppu_timer_delta{0},
    mode{PpuMode::OAM_SCAN},
    waiting{false},
    stat_blocking{false},
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
                sprite_buffer.clear();
                uint8_t ly = gbBus->read(LY_ADDR);
                uint8_t obj_size = (gbBus->read(LCDC_ADDR) & 0b00000100)? 16 : 8;
                uint16_t oam_index = 0xFE00;
                while(oam_index < 0xFE9F && sprite_buffer.size() < 10){
                    Sprite sprite = LoadSprite(oam_index);
                    if(sprite.x_pos > 0 && (ly + 16 >= sprite.y_pos) && (ly + 16 <= sprite.y_pos + obj_size)){
                        sprite_buffer.push_back(sprite);
                    }
                    oam_index += 4; //go to next entry
                }
                waiting = true;
            } else if(ppu_timer >= 80){
                ChangeModes(PpuMode::DRAWING);
            }
            break;
        case PpuMode::DRAWING:
            if(!waiting){
                //execute Draw to Line
                waiting = true;
            } else if(ppu_timer >= 172 + drawing_penalty){
                ChangeModes(PpuMode::HBLANK);
            }
            break;
        case PpuMode::HBLANK:
            if(!waiting){
                //wait
                waiting = true;
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
        if((gbBus->read(STAT_ADDR) & 0b01000000)){
            if (!stat_blocking){
                stat_blocking = true;
                gbBus->write((gbBus->read(IF_ADDR) | 0b00000010), IF_ADDR); //request lcd_stat interrupt
            }
        }
    } else {
        gbBus->write((gbBus->read(STAT_ADDR) & ~0b00000100), STAT_ADDR); //LYC == LY
    }
}

void GBPpu::ChangeModes(PpuMode mode){ 
    waiting = false;
    this->mode = mode;
    ppu_timer = 0;
    gbBus->write((gbBus->read(STAT_ADDR) & 0b11111100) | static_cast<uint8_t>(mode), STAT_ADDR);
    //logic for dispatching interrupts
    if (CheckCondition(mode)){ // check condition for current mode change
        if (!stat_blocking){
            stat_blocking = true;
            gbBus->write((gbBus->read(IF_ADDR) | 0b00000010), IF_ADDR); //request lcd_stat interrupt
        }
    } else if (mode != PpuMode::DRAWING){ // stat interrupt line goes low
        stat_blocking = false;
    }
}

bool GBPpu::CheckCondition(PpuMode mode){
    switch (mode)
    {
    case PpuMode::HBLANK: //mode 0
        return gbBus->read(STAT_ADDR) & 0b00001000;
        break;
    case PpuMode::VBLANK: //mode 1
        return gbBus->read(STAT_ADDR) & 0b00010000;
        break;
    case PpuMode::OAM_SCAN: //mode 2
        return gbBus->read(STAT_ADDR) & 0b00100000;
        break;
    
    default:
        return false;
        break;
    }
}

Sprite GBPpu::LoadSprite(uint16_t addr){
    return {
        gbBus->read(addr), //y_pos
        gbBus->read(addr + 1), //x_pos
        gbBus->read(addr + 2), //tile_index
        gbBus->read(addr + 3), //flags
    };
}