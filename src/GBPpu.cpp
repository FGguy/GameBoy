#include "GBPpu.h"

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
    display_buffer.resize(160 * 144);
}

void GBPpu::updateTimer(uint16_t cycles){
    if(!(gbBus->read(LCDC_ADDR) & 0b10000000)){ //PPU is disabled
        waiting = false;
        ppu_timer = 0;
        ppu_timer_delta = 0;
        mode = obj_attribute_mem_SCAN;
        gbBus->write(0, LY_ADDR);
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
                    if(sprite.x_pos > 0 && (ly + 16 >= sprite.y_pos) && (ly + 16 < sprite.y_pos + obj_size)){
                        sprite_buffer.push_back(sprite);
                    }
                    obj_attribute_mem_index += 4;
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
                waiting = true;
            } else if(ppu_timer >= 456U){
                uint8_t ly = gbBus->read(LY_ADDR);
                if(ly < 143){
                    gbBus->write(ly + 1, LY_ADDR);
                    changeModes(PpuMode::obj_attribute_mem_SCAN);
                } else {
                    gbBus->write(ly + 1, LY_ADDR);
                    changeModes(PpuMode::VBLANK);
                    gbBus->write((gbBus->read(IF_ADDR) | 0b00000001), IF_ADDR); // VBlank interrupt
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
                uint8_t ly = gbBus->read(LY_ADDR);
                if(ly < 153){
                    gbBus->write(ly + 1, LY_ADDR);
                }
                ppu_timer_delta = 0;
            }
            break;
    }
    
    // LYC == LY comparison
    if(gbBus->read(LY_ADDR) == gbBus->read(LYC_ADDR)){
        gbBus->write((gbBus->read(STAT_ADDR) | 0b00000100), STAT_ADDR);
        if((gbBus->read(STAT_ADDR) & 0b01000000)){
            if (!stat_blocking){
                stat_blocking = true;
                gbBus->write((gbBus->read(IF_ADDR) | 0b00000010), IF_ADDR);
            }
        }
    } else {
        gbBus->write((gbBus->read(STAT_ADDR) & ~0b00000100), STAT_ADDR);
    }
}

void GBPpu::changeModes(PpuMode mode){ 
    waiting = false;
    this->mode = mode;
    ppu_timer = 0;
    drawing_penalty = 0;
    gbBus->write((gbBus->read(STAT_ADDR) & 0b11111100) | static_cast<uint8_t>(mode), STAT_ADDR);
    
    if (checkCondition(mode)){
        if (!stat_blocking){
            stat_blocking = true;
            gbBus->write((gbBus->read(IF_ADDR) | 0b00000010), IF_ADDR);
        }
    } else if (mode != PpuMode::DRAWING){
        stat_blocking = false;
    }
}

bool GBPpu::checkCondition(PpuMode mode){
    switch (mode)
    {
    case PpuMode::HBLANK:
        return gbBus->read(STAT_ADDR) & 0b00001000;
    case PpuMode::VBLANK:
        return gbBus->read(STAT_ADDR) & 0b00010000;
    case PpuMode::obj_attribute_mem_SCAN:
        return gbBus->read(STAT_ADDR) & 0b00100000;
    default:
        return false;
    }
}

Sprite GBPpu::loadSprite(uint16_t addr){
    return {
        gbBus->read(addr),
        gbBus->read(addr + 1),
        gbBus->read(addr + 2),
        gbBus->read(addr + 3),
    };
}

std::vector<uint8_t> GBPpu::fetchTile(uint8_t tile_index, bool isSprite){
    std::vector<uint8_t> tile_data;
    uint16_t tile_data_addr;
    
    if(isSprite || (gbBus->read(LCDC_ADDR) & 0b00010000)){
        // Unsigned addressing: 0x8000-0x8FFF
        tile_data_addr = 0x8000 + (tile_index * 16);
    } else {
        // Signed addressing: 0x8800-0x97FF
        int8_t signed_index = static_cast<int8_t>(tile_index);
        tile_data_addr = 0x9000 + (signed_index * 16);
    }
    
    for(int i = 0; i < 16; i++){
        tile_data.push_back(gbBus->read(tile_data_addr + i));
    }
    
    return tile_data;
}

void GBPpu::drawingMode(){
    uint8_t ly = gbBus->read(LY_ADDR);
    uint8_t scx = gbBus->read(SCX_ADDR);
    uint8_t scy = gbBus->read(SCY_ADDR);
    uint8_t wx = gbBus->read(WX_ADDR);
    uint8_t wy = gbBus->read(WY_ADDR);
    uint8_t lcdc = gbBus->read(LCDC_ADDR);
    
    bool bg_enabled = lcdc & 0b00000001;
    bool window_enabled = (lcdc & 0b00100000) && (ly >= wy);
    bool sprites_enabled = lcdc & 0b00000010;
    
    // Calculate scrolling penalty
    drawing_penalty = scx % 8;
    
    // Clear FIFOs
    while(!background_FIFO.empty()) background_FIFO.pop();
    while(!sprite_FIFO.empty()) sprite_FIFO.pop();
    
    // Render 160 pixels for this scanline
    for(uint8_t x = 0; x < 160; x++){
        Pixel bg_pixel = {0, 0, 0};
        
        // Determine if we're in window region
        bool in_window = window_enabled && (x + 7 >= wx);
        
        if(bg_enabled || in_window){
            uint8_t tile_x, tile_y;
            uint16_t tilemap_addr;
            
            if(in_window){
                // Window rendering
                tile_x = (x + 7 - wx) / 8;
                tile_y = (ly - wy) / 8;
                tilemap_addr = (lcdc & 0b01000000) ? 0x9C00 : 0x9800;
                drawing_penalty += 6; // Window penalty
            } else {
                // Background rendering
                tile_x = ((x + scx) / 8) % 32;
                tile_y = ((ly + scy) / 8) % 32;
                tilemap_addr = (lcdc & 0b00001000) ? 0x9C00 : 0x9800;
            }
            
            uint16_t tile_addr = tilemap_addr + (tile_y * 32) + tile_x;
            uint8_t tile_index = gbBus->read(tile_addr);
            
            std::vector<uint8_t> tile_data = fetchTile(tile_index, false);
            
            uint8_t pixel_y = in_window ? ((ly - wy) % 8) : ((ly + scy) % 8);
            uint8_t pixel_x = in_window ? ((x + 7 - wx) % 8) : ((x + scx) % 8);
            
            uint8_t low_byte = tile_data[pixel_y * 2];
            uint8_t high_byte = tile_data[pixel_y * 2 + 1];
            
            uint8_t bit_pos = 7 - pixel_x;
            uint8_t color_bit_low = (low_byte >> bit_pos) & 0x01;
            uint8_t color_bit_high = (high_byte >> bit_pos) & 0x01;
            
            bg_pixel.color = (color_bit_high << 1) | color_bit_low;
            bg_pixel.palette = 0;
            bg_pixel.bg_priority = 0;
        }
        
        // Sprite rendering
        Pixel sprite_pixel = {0, 0, 0};
        bool sprite_found = false;
        
        if(sprites_enabled){
            uint8_t obj_size = (lcdc & 0b00000100) ? 16 : 8;
            
            for(const auto& sprite : sprite_buffer){
                int sprite_x = sprite.x_pos - 8;
                int sprite_y = sprite.y_pos - 16;
                
                if(x >= sprite_x && x < sprite_x + 8 && ly >= sprite_y && ly < sprite_y + obj_size){
                    std::vector<uint8_t> sprite_data = fetchTile(sprite.tile_index, true);
                    
                    uint8_t pixel_y = ly - sprite_y;
                    uint8_t pixel_x = x - sprite_x;
                    
                    // Handle Y flip
                    if(sprite.CheckFlag(SpriteFlags::Y_FLIP)){
                        pixel_y = (obj_size - 1) - pixel_y;
                    }
                    
                    // Handle X flip
                    if(sprite.CheckFlag(SpriteFlags::X_FLIP)){
                        pixel_x = 7 - pixel_x;
                    }
                    
                    uint8_t low_byte = sprite_data[pixel_y * 2];
                    uint8_t high_byte = sprite_data[pixel_y * 2 + 1];
                    
                    uint8_t bit_pos = 7 - pixel_x;
                    uint8_t color_bit_low = (low_byte >> bit_pos) & 0x01;
                    uint8_t color_bit_high = (high_byte >> bit_pos) & 0x01;
                    uint8_t color = (color_bit_high << 1) | color_bit_low;
                    
                    if(color != 0){ // Not transparent
                        sprite_pixel.color = color;
                        sprite_pixel.palette = sprite.CheckFlag(SpriteFlags::PALETTE);
                        sprite_pixel.bg_priority = sprite.CheckFlag(SpriteFlags::PRIORITY);
                        sprite_found = true;
                        drawing_penalty += 6; // Sprite penalty
                        break; // Use first non-transparent sprite
                    }
                }
            }
        }
        
        // Mix background and sprite pixels
        Pixel final_pixel;
        
        if(sprite_found && sprite_pixel.color != 0){
            // Check priority
            if(sprite_pixel.bg_priority && bg_pixel.color != 0){
                final_pixel = bg_pixel;
            } else {
                final_pixel = sprite_pixel;
            }
        } else {
            final_pixel = bg_pixel;
        }
        
        // Apply palette and store in display buffer
        uint8_t palette_data;
        if(sprite_found && final_pixel.color == sprite_pixel.color){
            // Sprite palette
            palette_data = gbBus->read(final_pixel.palette ? OBP1_ADDR : OBP0_ADDR);
        } else {
            // Background palette
            palette_data = gbBus->read(BGP_ADDR);
        }
        
        uint8_t color_value = (palette_data >> (final_pixel.color * 2)) & 0x03;
        display_buffer[ly * 160 + x] = Pixel{color_value, 0, 0};
    }
}