#include <GBPpu.h>

GBPpu::GBPpu(GBBus* gbBus):
    gbBus{gbBus},
    sprite_buffer(10),
    ppu_timer{0}
{
}