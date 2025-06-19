#pragma once
#include <GBBus.h>

#include <vector>

/*
    Tiles: 8x8 pixels, each pixel is 2 bits so each tiles is 16 bytes
    2 bytes -> 1 row of 8 pixels, 1st byte is least sig bit 2nd is most sig bit

    3 layers: Background, Window and Sprites
    Background is 32x32 tiles (256x256 pixels) meanwhile display is 160x144
    Viewport is defined using scroll registers SCX & SCY
    Window is also 32x32 tiles
    Overlay position is defined by registers WX & WY 
    The VRAM sections $9800-$9BFF and $9C00-$9FFF each contain a background map

    Sprites are 1 tile and are stored their data is stored in OAM
    byte 0: Y-position
    byte 1: X-position
    byte 2: Tile Number (always 8000 addressing method)
    byte 3: sprite flags

    Tiles are stored within regions of VRAM and are indexed using different methods 
    depending on bit 4 of the LCDC register which sets the addressing mode

    The PPU operates on scanlines, there is 154 scanlines (144 actual + 10 Vblank)
    The scanline currently being processed is indicated in register LY

    4 modes:
    *each the PPU changes mode, the lower two bits of the STAT register are updated. 

    OAM Scan (Mode 2) 80 T-cycles:
        entered at the beginning of every scanline, PPU scans OAM for sprites to render.
        Sprites to be rendered are added to a buffer.
    
    Drawing (Mode 3) 172 - 289 T-cycles:
        pixels are drawn to the line

    Hblank (Mode 0) 87 - 204 T-cycles:
        padding period, nothing is done

    Vblank (Mode 1) 456 T-cycles X 10:
        PPU does not do any work during this time
        CPU can access VRAM
*/

enum SpriteFlags {
    PALETTE = 0,
    X_FLIP,
    Y_FLIP,
    PRIORITY
};

struct Sprite {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t flags;

    uint8_t CheckFlag(SpriteFlags s_flag){
        // 1 is set 0 is unset
        return ((flags >> 4) >> s_flag) & 0x01;
    }
};

enum PpuMode {
    OAM_SCAN = 2,
    DRAWING = 3,
    HBLANK = 0,
    VBLANK = 1
};

class GBPpu {
    private:
        std::vector<Sprite> sprite_buffer;
        uint32_t ppu_timer;
        uint32_t ppu_timer_delta;
        PpuMode mode;
        bool waiting;
        uint8_t drawing_penalty;

        GBBus* gbBus;
    public:
        //matrix for pixels
        GBPpu(GBBus* gbBus);
        std::vector<uint8_t> FetchTile(uint8_t tile_index, bool isSprite);
        void UpdateTimer(uint8_t cycles);
        void ChangeModes(PpuMode mode);
};