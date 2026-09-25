#include "xtemu.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* CPU instruction set */
typedef enum {
    OP_NOP = 0x00,
    OP_JMP = 0xE9,
    OP_CALL = 0xE8,
    OP_RET = 0xC3,
    OP_INT = 0xCD,
    OP_PUSH = 0x50,
    OP_POP = 0x58,
    OP_MOV = 0xB8,
    OP_ADD = 0x04,
    OP_SUB = 0x2C,
    OP_CMP = 0x3C,
    OP_OR = 0x0C,
    OP_AND = 0x24,
    OP_XOR = 0x34,
    OP_INC = 0x40,
    OP_DEC = 0x48,
    OP_LOOP = 0xE2,
    OP_JZ = 0x74,
    OP_JNZ = 0x75,
    OP_IN = 0xE4,
    OP_OUT = 0xE6,
    OP_STI = 0xFB,
    OP_CLI = 0xFA
} xt_opcode_t;

/* Initialize emulator */
int xt_init(xt_emulator_t *emu) {
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    
    if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL timer initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    
    /* Initialize display */
    emu->display.window = SDL_CreateWindow("xtemu - IBM XT Emulator",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          XT_VGA_WIDTH, XT_VGA_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!emu->display.window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        return -1;
    }
    
    emu->display.renderer = SDL_CreateRenderer(emu->display.window, -1, SDL_RENDERER_ACCELERATED);
    if (!emu->display.renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        return -1;
    }
    
    emu->display.texture = SDL_CreateTexture(emu->display.renderer,
                                           SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           XT_VGA_WIDTH, XT_VGA_HEIGHT);
    if (!emu->display.texture) {
        fprintf(stderr, "Texture creation failed: %s\n", SDL_GetError());
        return -1;
    }
    
    emu->display.framebuffer = malloc(XT_VGA_WIDTH * XT_VGA_HEIGHT * 4);
    if (!emu->display.framebuffer) {
        fprintf(stderr, "Framebuffer allocation failed\n");
        return -1;
    }
    
    emu->display.initialized = true;
    
    /* Initialize keyboard */
    memset(&emu->keyboard, 0, sizeof(emu->keyboard));
    
    /* Initialize memory */
    memset(emu->memory.ram, 0, sizeof(emu->memory.ram));
    memset(emu->memory.video_ram, 0, sizeof(emu->memory.video_ram));
    
    /* Load BIOS ROM (placeholder - will load actual BIOS later) */
    // TODO: Load actual IBM PC BIOS ROM
    memset(emu->memory.bios_rom, 0x90, sizeof(emu->memory.bios_rom)); // NOPs for now
    
    /* Reset CPU state */
    xt_reset(emu);
    
    emu->running = false;
    emu->debug_mode = false;
    
    return 0;
}

/* Cleanup emulator */
void xt_cleanup(xt_emulator_t *emu) {
    if (emu->display.texture) {
        SDL_DestroyTexture(emu->display.texture);
    }
    if (emu->display.renderer) {
        SDL_DestroyRenderer(emu->display.renderer);
    }
    if (emu->display.window) {
        SDL_DestroyWindow(emu->display.window);
    }
    if (emu->display.framebuffer) {
        free(emu->display.framebuffer);
    }
    
    SDL_Quit();
}

/* Reset emulator state */
void xt_reset(xt_emulator_t *emu) {
    /* Reset CPU registers */
    memset(&emu->cpu, 0, sizeof(emu->cpu));
    emu->cpu.ax = 0;
    emu->cpu.bx = 0;
    emu->cpu.cx = 0;
    emu->cpu.dx = 0;
    emu->cpu.si = 0;
    emu->cpu.di = 0;
    emu->cpu.bp = 0;
    emu->cpu.sp = 0xFFFE;
    emu->cpu.cs = 0xF000;
    emu->cpu.ds = 0;
    emu->cpu.es = 0;
    emu->cpu.ss = 0;
    emu->cpu.ip = 0xFFF0; /* Start at BIOS entry point */
    emu->cpu.flags = 0x0002; /* IF=0 (interrupts disabled) */
    emu->cpu.cycles = 0;
    emu->cpu.turbo_mode = false;
    
    /* Reset memory */
    memset(emu->memory.ram, 0, sizeof(emu->memory.ram));
    
    /* Reset display */
    if (emu->display.framebuffer) {
        memset(emu->display.framebuffer, 0, XT_VGA_WIDTH * XT_VGA_HEIGHT * 4);
    }
}

/* Execute one CPU instruction */
void xt_step(xt_emulator_t *emu) {
    uint16_t cs_ip = (emu->cpu.cs << 4) + emu->cpu.ip;
    uint8_t opcode = emu->memory.bios_rom[cs_ip];
    
    switch (opcode) {
        case OP_NOP:
            /* No operation */
            emu->cpu.ip += 1;
            emu->cpu.cycles += 4;
            break;
            
        case OP_JMP:
            /* Jump (short) */
            emu->cpu.ip += 2;
            emu->cpu.cycles += 15;
            break;
            
        case OP_RET:
            /* Return */
            emu->cpu.ip += 1;
            emu->cpu.cycles += 20;
            break;
            
        case OP_INT:
            /* Interrupt */
            emu->cpu.ip += 2;
            emu->cpu.cycles += 52;
            break;
            
        default:
            /* Unknown instruction */
            printf("Unknown opcode: 0x%02X at CS:IP %04X:%04X\n", 
                   opcode, emu->cpu.cs, emu->cpu.ip);
            emu->cpu.ip += 1;
            emu->cpu.cycles += 4;
            break;
    }
}

/* Main emulation loop */
void xt_run(xt_emulator_t *emu) {
    emu->running = true;
    
    while (emu->running) {
        /* Handle SDL events */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    emu->running = false;
                    break;
                case SDL_KEYDOWN:
                    /* Handle keyboard input */
                    break;
                case SDL_KEYUP:
                    /* Handle keyboard release */
                    break;
            }
        }
        
        /* Execute CPU instruction */
        xt_step(emu);
        
        /* Update display */
        if (emu->display.initialized) {
            SDL_UpdateTexture(emu->display.texture, NULL, 
                            emu->display.framebuffer, 
                            XT_VGA_WIDTH * 4);
            SDL_RenderClear(emu->display.renderer);
            SDL_RenderCopy(emu->display.renderer, emu->display.texture, NULL, NULL);
            SDL_RenderPresent(emu->display.renderer);
        }
        
        /* Control emulation speed */
        if (emu->cpu.turbo_mode) {
            /* Turbo mode - run faster */
            SDL_Delay(1);
        } else {
            /* Normal mode - maintain XT speed */
            struct timespec ts = {0, 1000000}; /* 1ms */
            nanosleep(&ts, NULL);
        }
    }
}

int main(int argc, char *argv[]) {
    xt_emulator_t emu;
    
    printf("xtemu - IBM XT Emulator\n");
    printf("Initializing...\n");
    
    if (xt_init(&emu) < 0) {
        fprintf(stderr, "Failed to initialize emulator\n");
        return 1;
    }
    
    printf("Starting emulation...\n");
    xt_run(&emu);
    
    printf("Cleaning up...\n");
    xt_cleanup(&emu);
    
    return 0;
}