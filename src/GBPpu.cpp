#include <GBPpu.h>

/*
TODO: The actual graphical rendering pipeline lol
TODO: obj_attribute_mem DMA transfers

bits 6-3 are used to enable interrupts caused by a transition to a certain mode
STAT interrupt only caused by rising edge on interrupt line
what causes the interrupt line to reset, when are the bits set low?
No interrupt is dispatched while a condition is still being met

mode change? Lyc == Ly

if yes check condition bit 

if yes raise interrupt

set stat_blocking to true

stat_blocking to set to false by a check that evaluates to false

Drawing:
    Pixel buffer for screen stored in PPU
    2 queues for sprites and background
    push pixels to buffer in drawing
    draw frame during Vblank and clear buffer

*/

GBPpu::GBPpu(GBBus* gbBus):
    gbBus{gbBus},
    sprite_buffer(10),
    ppu_timer{0},
    ppu_timer_delta{0},
    mode{PpuMode::obj_attribute_mem_SCAN},
    waiting{false},
    stat_blocking{false},
    drawing_penalty{0}
{
}

void GBPpu::updateTimer(uint16_t cycles){
    if(gbBus->read(LCDC_ADDR) & 0b10000000){ //PPU is disabled
        waiting = false;
        ppu_timer = 0;
        ppu_timer_delta = 0;
        mode = obj_attribute_mem_SCAN;
        return;
    }
    ppu_timer += cycles;
    switch(mode){
        case PpuMode::obj_attribute_mem_SCAN:
            if(!waiting){
                sprite_buffer.clear();
                uint8_t ly = gbBus->read(LY_ADDR);
                uint8_t obj_size = (gbBus->read(LCDC_ADDR) & 0b00000100)? 16 : 8;
                uint16_t obj_attribute_mem_index = 0xFE00;
                while(obj_attribute_mem_index < 0xFE9F && sprite_buffer.size() < 10){
                    Sprite sprite = loadSprite(obj_attribute_mem_index);
                    if(sprite.x_pos > 0 && (ly + 16 >= sprite.y_pos) && (ly + 16 <= sprite.y_pos + obj_size)){
                        sprite_buffer.push_back(sprite);
                    }
                    obj_attribute_mem_index += 4; //go to next entry
                }
                waiting = true;
            } else if(ppu_timer >= 80){
                changeModes(PpuMode::DRAWING);
            }
            break;
        case PpuMode::DRAWING:
            if(!waiting){
                drawingMode();
                waiting = true;
            } else if(ppu_timer >= 172U + drawing_penalty){
                changeModes(PpuMode::HBLANK);
            }
            break;
        case PpuMode::HBLANK:
            if(!waiting){
                //wait
                waiting = true;
            } else if(ppu_timer >= 204U - drawing_penalty){
                uint8_t ly = gbBus->read(LY_ADDR);
                if(ly < 144){
                    changeModes(PpuMode::obj_attribute_mem_SCAN);
                    gbBus->write(ly + 1, LY_ADDR);
                } else {
                    changeModes(PpuMode::VBLANK);
                }
            }
            break;
        case PpuMode::VBLANK:
            ppu_timer_delta += cycles;
            if(ppu_timer >= 4560){
                changeModes(PpuMode::obj_attribute_mem_SCAN);
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

void GBPpu::changeModes(PpuMode mode){ 
    waiting = false;
    this->mode = mode;
    ppu_timer = 0;
    gbBus->write((gbBus->read(STAT_ADDR) & 0b11111100) | static_cast<uint8_t>(mode), STAT_ADDR);
    //logic for dispatching interrupts
    if (checkCondition(mode)){ // check condition for current mode change
        if (!stat_blocking){
            stat_blocking = true;
            gbBus->write((gbBus->read(IF_ADDR) | 0b00000010), IF_ADDR); //request lcd_stat interrupt
        }
    } else if (mode != PpuMode::DRAWING){ // stat interrupt line goes low
        stat_blocking = false;
    }
}

bool GBPpu::checkCondition(PpuMode mode){
    switch (mode)
    {
    case PpuMode::HBLANK: //mode 0
        return gbBus->read(STAT_ADDR) & 0b00001000;
        break;
    case PpuMode::VBLANK: //mode 1
        return gbBus->read(STAT_ADDR) & 0b00010000;
        break;
    case PpuMode::obj_attribute_mem_SCAN: //mode 2
        return gbBus->read(STAT_ADDR) & 0b00100000;
        break;
    
    default:
        return false;
        break;
    }
}

Sprite GBPpu::loadSprite(uint16_t addr){
    return {
        gbBus->read(addr), //y_pos
        gbBus->read(addr + 1), //x_pos
        gbBus->read(addr + 2), //tile_index
        gbBus->read(addr + 3), //flags
    };
}

//TODO: be able to change the values of SCX, SCY, WY, WX mid scanline 
inline void GBPpu::drawingMode(){
    /*
        Pushes one row of pixels (160 pixels) to the display_buffer

        Fetch tile:
            get tile number of current tile
            depends on if rendering background or window and LCDC. 3 & 5
        Fetch tile data high:
        Fetch tile data low:
        Push to FIFO:
            decode fetched tile data into pixels and push to pixel queue
        Push to LCD:
            using algorithm clear the FIFOs into to LCD

        Repeat until line is finished (160 pixels pushed to LCD)
        
    */
   uint8_t x_position_counter = 0;

    //how do sprite and background fetch interact with eachother
}