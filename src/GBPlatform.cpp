#include <GBPlatform.h>

GBPlatform::GBPlatform(std::vector<std::uint8_t>& bootROM, std::vector<std::uint8_t>& cartridgeROM):
    bootROM{bootROM},
    cartridgeROM{cartridgeROM},
    gbBus{bootROM, cartridgeROM},
    gbCpu{&gbBus},
    gbPpu{&gbBus},
    quit{false}
{
}

int GBPlatform::BootAndExecute(){
    //start window setup
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { //add error message if fails
        return 1;
    }

    gbWindow = SDL_CreateWindow("GameBoy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 640, SDL_WINDOW_SHOWN);
    if (!gbWindow) {
        SDL_Quit();
        return 1;
    }

    gbRenderer = SDL_CreateRenderer(gbWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gbRenderer) {
        SDL_DestroyWindow(gbWindow);
        SDL_Quit();
        return 1;
    }

    gbBus.write(0x00, BOOTLOCK_ADDR); //enabled boot rom

    //VBLANK 59.7 times/s DONE
    //STAT/LCD 
    //TIMER everytime the timer overflows DONE
    //Serial will not be implemented TECHNICALLY DONE
    //JOYPAD anytime one of the bits in P1 changes DONE

    while(!quit){
        std::uint16_t cycles{0};
        cycles += gbCpu.decodeExecuteInstruction();
        cycles += gbCpu.handleInterrupts();
        gbPpu.UpdateTimer(cycles);
        UpdateTimers(cycles);
        ProcessInputs();
    }

    //cleanup
    SDL_DestroyRenderer(gbRenderer);
    SDL_DestroyWindow(gbWindow);
    SDL_Quit();
    return 0;
}

//uses M-cycles 1.048576 MHz
inline void GBPlatform::UpdateTimers(std::uint16_t cycles){
    vblank_timer += cycles; //59.7 times a sec, every 17564 M-Cycles
    if (vblank_timer >= 17564){
        gbCpu.requestInterrupt(VBlank);
        vblank_timer = 0;
        RenderFrame();
    }
    lcd_timer += cycles;
    
    std::uint8_t tac = gbBus.read(TAC_ADDR);
    if(tac & 0b00000100){
        tima_timer += cycles; //incremented based on tac reg 00: 4096M, 01: 262144M, 10: 65536M 11: 16384, bit 2 is checked to see if timer is enabled, is reset to TMA value
        std::uint8_t tma = gbBus.read(TMA_ADDR);
        switch (tac & 0b00000011)
        {
        case 0b00000000:
            if (tima_timer >= 4096){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                tima_timer = 0;
            }
            break;

        case 0b00000001:
            if (tima_timer >= 262144){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                tima_timer = 0;
            }
            break;

        case 0b00000010:
            if (tima_timer >= 65536){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                tima_timer = 0;
            }
            break;

        case 0b00000011:
            if (tima_timer >= 16384){
                gbBus.write(gbBus.read(TIMA_ADDR) + 1, TIMA_ADDR);
                tima_timer = 0;
            }
            break;
        }
        if (gbBus.read(TIMA_ADDR) == 0){
            gbCpu.requestInterrupt(Timer);
        }
    }
    div_timer += cycles; //16384Hz, writing to it resets to 0, resets during stop, is not incremente during stop.
    if (div_timer >= 64){
        gbBus.write(gbBus.read(DIV_ADDR) + 1, DIV_ADDR);
    }

    gbBus.write(gbBus.read(DIV_ADDR) + 1,DIV_ADDR);
}

void GBPlatform::ProcessInputs(){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_QUIT: {
                quit = true;
            } break;
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym){
                    case SDLK_a:
                        b_left = 0;
                        break;
                    case SDLK_w:
                        b_up = 0;
                        break;
                    case SDLK_s:
                        b_down = 0;
                        break;
                    case SDLK_d:
                        b_right = 0;
                        break;
                    case SDLK_j:
                        b_B = 0;
                        break;
                    case SDLK_k:
                        b_A = 0;
                        break;
                    case SDLK_v:
                        b_select = 0;
                        break;
                    case SDLK_b:
                        b_start = 0;
                        break;
                }
            //request interrupt 
            gbCpu.requestInterrupt(Joypad);
            } break;
            case SDL_KEYUP: {
                switch (event.key.keysym.sym){
                    case SDLK_a:
                        b_left = 1;
                        break;
                    case SDLK_w:
                        b_up = 1;
                        break;
                    case SDLK_s:
                        b_down = 1;
                        break;
                    case SDLK_d:
                        b_right = 1;
                        break;
                    case SDLK_j:
                        b_B = 1;
                        break;
                    case SDLK_k:
                        b_A = 1;
                        break;
                    case SDLK_v:
                        b_select = 1;
                        break;
                    case SDLK_b:
                        b_start = 1;
                        break;
                }
            } break;
        }
    }
    //map new inputs to IO register and request interrupt
    if((~gbBus.read(JOYP_ADDR) & 0b00010000)){ //d-pad
        gbBus.write((0b00100000 | b_down << 3 | b_up << 2 | b_left << 1 | b_right), JOYP_ADDR);
    } else if((~gbBus.read(JOYP_ADDR) & 0b00100000)){ // select buttons
        gbBus.write((0b00010000 | b_start << 3 | b_select << 2 | b_B << 1 | b_A), JOYP_ADDR);
    }
}

void GBPlatform::RenderFrame(){
    /*
        TODO: From pixel matrix in PPU, render a frame and display it using SDL
    */
}