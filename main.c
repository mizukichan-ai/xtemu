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
    OP_CLI = 0xFA,
    /* Additional opcodes for Phase 2 */
    OP_ADC = 0x14,
    OP_SBB = 0x1C,
    OP_DAA = 0x27,
    OP_AAS = 0x2F,
    OP_XCHG = 0x87,
    OP_CBW = 0x98,
    OP_CWD = 0x99,
    OP_SAL = 0xC0,
    OP_SHL = 0xC0,
    OP_SAR = 0xC0,
    OP_SHR = 0xC0,
    OP_ROL = 0xC0,
    OP_ROR = 0xC0,
    OP_RCL = 0xC0,
    OP_RCR = 0xC0,
    OP_TEST = 0xF6,
    OP_NEG = 0xF6,
    OP_MUL = 0xF6,
    OP_IMUL = 0xF6,
    OP_DIV = 0xF6,
    OP_IDIV = 0xF6,
    OP_MOVSB = 0xA4,
    OP_MOVSW = 0xA5,
    OP_CMPSB = 0xA6,
    OP_CMPSW = 0xA7,
    OP_SCASB = 0xAE,
    OP_SCASW = 0xAF,
    OP_LODSB = 0xAC,
    OP_LODSW = 0xAD,
    OP_STOSB = 0xAA,
    OP_STOSW = 0xAB,
    OP_STC = 0xF9,
    OP_CLC = 0xF8,
    OP_CMC = 0xF5,
    OP_STD = 0xFD,
    OP_CLD = 0xFC,
    OP_LAHF = 0x9F,
    OP_SAHF = 0x9E,
    OP_LOOPNE = 0xE0,
    OP_LOOPE = 0xE1,
    OP_JCXZ = 0xE3
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
    xt_display_init(&emu->display);
    
    if (!emu->display.initialized) {
        fprintf(stderr, "Display initialization failed\n");
        return -1;
    }
    
    /* Create SDL window and renderer (this will be moved to display.c later) */
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
    
    /* Initialize display with default video mode */
    xt_display_set_video_mode(&emu->display, XT_VIDEO_MODE_TEXT_80x25);
    
    /* Initialize keyboard */
    xt_keyboard_init(&emu->keyboard);
    
    /* Initialize PIC */
    xt_pic_init(&emu->pic);
    
    /* Initialize PIT */
    xt_pit_init(&emu->pit);
    
    /* Initialize DMA */
    xt_dma_init(&emu->dma);
    
    /* Initialize memory */
    xt_memory_init(&emu->memory);
    
    /* Load BIOS ROM */
    if (xt_memory_load_bios(&emu->memory, "bios.bin") < 0) {
        printf("Warning: Using NOP BIOS ROM\n");
    }
    
    /* Reset CPU state */
    xt_reset(emu);
    
    emu->running = false;
    emu->debug_mode = false;
    
    return 0;
}

/* Cleanup emulator */
void xt_cleanup(xt_emulator_t *emu) {
    /* Cleanup display */
    xt_display_cleanup(&emu->display);
    
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
    emu->cpu.sp = 0xFFFE; /* Stack pointer at top of segment */
    emu->cpu.cs = 0xF000; /* Code segment at BIOS ROM */
    emu->cpu.ds = 0x0000; /* Data segment at conventional RAM */
    emu->cpu.es = 0x0000; /* Extra segment at conventional RAM */
    emu->cpu.ss = 0x0000; /* Stack segment at conventional RAM */
    emu->cpu.ip = 0xFFF0; /* Start at BIOS entry point */
    emu->cpu.flags = 0x0002; /* IF=0 (interrupts disabled) */
    emu->cpu.cycles = 0;
    emu->cpu.turbo_mode = false;
    
    /* Reset memory */
    xt_memory_init(&emu->memory);
    
    /* Reset display */
    if (emu->display.framebuffer) {
        memset(emu->display.framebuffer, 0, XT_VGA_WIDTH * XT_VGA_HEIGHT * 4);
    }
    
    printf("Emulator reset:\n");
    printf("- CS:IP = %04X:%04X\n", emu->cpu.cs, emu->cpu.ip);
    printf("- DS:ES:SS = %04X:%04X:%04X\n", emu->cpu.ds, emu->cpu.es, emu->cpu.ss);
    printf("- Stack: SS=0x%04X, SP=0x%04X\n", emu->cpu.ss, emu->cpu.sp);
}

/* Execute one CPU instruction */
void xt_step(xt_emulator_t *emu) {
    uint32_t cs_ip = xt_memory_segment_to_linear(emu->cpu.cs, emu->cpu.ip);
    uint8_t opcode = xt_memory_read_byte(&emu->memory, cs_ip);
    
    printf("Executing opcode: 0x%02X at CS:IP %04X:%04X (linear: 0x%05X)\n", 
           opcode, emu->cpu.cs, emu->cpu.ip, cs_ip);
           
    switch (opcode) {
        case 0x90: /* NOP */
            /* No operation */
            emu->cpu.ip += 1;
            emu->cpu.cycles += 4;
            break;
            
        case 0xE9: /* JMP near */
            /* Jump to relative address */
            uint8_t offset = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            emu->cpu.ip += 2 + (int8_t)offset;
            emu->cpu.cycles += 15;
            break;
            
        case 0xEB: /* JMP short */
            /* Short jump */
            uint8_t short_offset = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            emu->cpu.ip += 2 + (int8_t)short_offset;
            emu->cpu.cycles += 12;
            break;
            
        case 0xC3: /* RET */
            /* Return from subroutine */
            if (!xt_memory_validate_stack_segment(&emu->cpu, emu->cpu.ss, emu->cpu.sp)) {
                printf("Invalid stack segment for RET\n");
                break;
            }
            emu->cpu.ip += 1;
            emu->cpu.cycles += 20;
            break;
            
        case 0xE8: /* CALL near */
            /* Call subroutine */
            uint8_t call_offset = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            uint16_t call_target = cs_ip + 2 + (int8_t)call_offset;
            
            /* Push return address onto stack */
            if (!xt_memory_validate_stack_segment(&emu->cpu, emu->cpu.ss, emu->cpu.sp) ||
                !xt_memory_check_stack_bounds(&emu->cpu, 2)) {
                printf("Stack error during CALL\n");
                break;
            }
            
            emu->cpu.sp -= 2;
            uint16_t *stack_ptr = (uint16_t*)(emu->memory.ram + emu->cpu.ss + emu->cpu.sp);
            *stack_ptr = emu->cpu.ip + 2;
            emu->cpu.ip = call_target;
            emu->cpu.cycles += 19;
            break;
            
        case 0xCD: /* INT n */
            /* Interrupt */
            uint8_t int_num = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            emu->cpu.ip += 2;
            emu->cpu.cycles += 52;
            printf("INT 0x%02X called at CS:IP %04X:%04X\n", int_num, emu->cpu.cs, emu->cpu.ip);
            break;
            
        case 0xFA: /* CLI */
            /* Clear interrupt flag */
            emu->cpu.ip += 1;
            emu->cpu.flags &= ~0x0002;
            emu->cpu.cycles += 4;
            break;
            
        case 0xFB: /* STI */
            /* Set interrupt flag */
            emu->cpu.ip += 1;
            emu->cpu.flags |= 0x0002;
            emu->cpu.cycles += 4;
            break;
            
        case 0xF0: /* LOCK prefix */
            /* Lock prefix - ignore for now */
            emu->cpu.ip += 1;
            emu->cpu.cycles += 2;
            break;
            
        case 0xE4: /* IN AL,imm8 */
            /* Input from port to AL */
            uint8_t in_port = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            uint8_t in_value = 0xFF; /* Default value - will be overridden by actual hardware */
            
            /* Handle specific ports */
            if (in_port == XT_PIC_MASTER || in_port == XT_PIC_MASTER + 1) {
                /* Read PIC register */
                in_value = xt_pic_read(&emu->pic, in_port);
            } else if (in_port >= XT_PIT_BASE && in_port <= XT_PIT_BASE + 3) {
                /* Read PIT register */
                in_value = xt_pit_read(&emu->pit, in_port);
            } else if (in_port == XT_KEYBOARD) {
                /* Read keyboard controller data port */
                in_value = xt_keyboard_read(&emu->keyboard, in_port);
            } else if (in_port < 16) {
                /* Read DMA register */
                in_value = xt_dma_read(&emu->dma, in_port);
            } else {
                printf("IN from port 0x%02X (AL = 0x%02X)\n", in_port, in_value);
            }
            
            emu->cpu.ax = (emu->cpu.ax & 0xFF00) | in_value;
            emu->cpu.ip += 2;
            emu->cpu.cycles += 10;
            break;
            
        case 0xE5: /* IN AX,imm8 */
            /* Input from port to AX */
            uint8_t in_port_ax = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            uint16_t in_value_ax = 0xFFFF; /* Default value - will be overridden by actual hardware */
            
            /* Handle specific ports */
            if (in_port_ax == XT_PIC_MASTER || in_port_ax == XT_PIC_MASTER + 1) {
                /* Read PIC register (16-bit read) */
                uint8_t low = xt_pic_read(&emu->pic, in_port_ax);
                uint8_t high = xt_pic_read(&emu->pic, in_port_ax + 1);
                in_value_ax = (high << 8) | low;
            } else if (in_port_ax >= XT_PIT_BASE && in_port_ax <= XT_PIT_BASE + 3) {
                /* Read PIT register (16-bit read) */
                uint8_t low = xt_pit_read(&emu->pit, in_port_ax);
                uint8_t high = xt_pit_read(&emu->pit, in_port_ax + 1);
                in_value_ax = (high << 8) | low;
            } else if (in_port_ax == XT_KEYBOARD) {
                /* Read keyboard controller data port (16-bit read) */
                uint8_t low = xt_keyboard_read(&emu->keyboard, in_port_ax);
                uint8_t high = xt_keyboard_read(&emu->keyboard, in_port_ax + 1);
                in_value_ax = (high << 8) | low;
            } else if (in_port_ax < 16) {
                /* Read DMA register (16-bit read) */
                uint8_t low = xt_dma_read(&emu->dma, in_port_ax);
                uint8_t high = xt_dma_read(&emu->dma, in_port_ax + 1);
                in_value_ax = (high << 8) | low;
            } else {
                printf("IN from port 0x%02X (AX = 0x%04X)\n", in_port_ax, in_value_ax);
            }
            
            emu->cpu.ax = in_value_ax;
            emu->cpu.ip += 2;
            emu->cpu.cycles += 10;
            break;
            
        case 0xE6: /* OUT imm8,AL */
            /* Output AL to port */
            uint8_t out_port = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            uint8_t out_value = emu->cpu.ax & 0xFF;
            
            /* Handle specific ports */
            if (out_port == XT_PIC_MASTER || out_port == XT_PIC_MASTER + 1) {
                /* Write to PIC register */
                xt_pic_write(&emu->pic, out_port, out_value);
            } else if (out_port >= XT_PIT_BASE && out_port <= XT_PIT_BASE + 3) {
                /* Write to PIT register */
                xt_pit_write(&emu->pit, out_port, out_value);
            } else if (out_port == XT_KEYBOARD) {
                /* Write to keyboard controller data port */
                xt_keyboard_write(&emu->keyboard, out_port, out_value);
            } else if (out_port < 16) {
                /* Write to DMA register */
                xt_dma_write(&emu->dma, out_port, out_value);
            } else {
                printf("OUT to port 0x%02X (value 0x%02X)\n", out_port, out_value);
            }
            
            emu->cpu.ip += 2;
            emu->cpu.cycles += 10;
            break;
            
        case 0xE7: /* OUT imm8,AX */
            /* Output AX to port */
            uint8_t out_port_ax = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            uint16_t out_value_ax = emu->cpu.ax;
            
            /* Handle specific ports */
            if (out_port_ax == XT_PIC_MASTER || out_port_ax == XT_PIC_MASTER + 1) {
                /* Write to PIC register (16-bit write) */
                xt_pic_write(&emu->pic, out_port_ax, out_value_ax & 0xFF);
                xt_pic_write(&emu->pic, out_port_ax + 1, (out_value_ax >> 8) & 0xFF);
            } else if (out_port_ax >= XT_PIT_BASE && out_port_ax <= XT_PIT_BASE + 3) {
                /* Write to PIT register (16-bit write) */
                xt_pit_write(&emu->pit, out_port_ax, out_value_ax & 0xFF);
                xt_pit_write(&emu->pit, out_port_ax + 1, (out_value_ax >> 8) & 0xFF);
            } else if (out_port_ax == XT_KEYBOARD) {
                /* Write to keyboard controller data port (16-bit write) */
                xt_keyboard_write(&emu->keyboard, out_port_ax, out_value_ax & 0xFF);
                xt_keyboard_write(&emu->keyboard, out_port_ax + 1, (out_value_ax >> 8) & 0xFF);
            } else if (out_port_ax < 16) {
                /* Write to DMA register (16-bit write) */
                xt_dma_write(&emu->dma, out_port_ax, out_value_ax & 0xFF);
                xt_dma_write(&emu->dma, out_port_ax + 1, (out_value_ax >> 8) & 0xFF);
            } else {
                printf("OUT to port 0x%02X (value 0x%04X)\n", out_port_ax, out_value_ax);
            }
            
            emu->cpu.ip += 2;
            emu->cpu.cycles += 10;
            break;
            
        case 0xEC: /* IN AL,DX */
            /* Input from port (DX) to AL */
            uint16_t dx_port = emu->cpu.dx;
            uint8_t dx_in_value = 0xFF; /* Default value - will be overridden by actual hardware */
            
            /* Handle specific ports */
            if (dx_port == XT_PIC_MASTER || dx_port == XT_PIC_MASTER + 1) {
                /* Read PIC register */
                dx_in_value = xt_pic_read(&emu->pic, dx_port);
            } else if (dx_port >= XT_PIT_BASE && dx_port <= XT_PIT_BASE + 3) {
                /* Read PIT register */
                dx_in_value = xt_pit_read(&emu->pit, dx_port);
            } else if (dx_port == XT_KEYBOARD) {
                /* Read keyboard controller data port */
                dx_in_value = xt_keyboard_read(&emu->keyboard, dx_port);
            } else if (dx_port < 16) {
                /* Read DMA register */
                dx_in_value = xt_dma_read(&emu->dma, dx_port);
            } else {
                printf("IN from port DX=0x%04X (AL = 0x%02X)\n", dx_port, dx_in_value);
            }
            
            emu->cpu.ax = (emu->cpu.ax & 0xFF00) | dx_in_value;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 10;
            break;
            
        case 0xED: /* IN AX,DX */
            /* Input from port (DX) to AX */
            uint16_t dx_port_ax = emu->cpu.dx;
            uint16_t dx_in_value_ax = 0xFFFF; /* Default value - will be overridden by actual hardware */
            
            /* Handle specific ports */
            if (dx_port_ax == XT_PIC_MASTER || dx_port_ax == XT_PIC_MASTER + 1) {
                /* Read PIC register (16-bit read) */
                uint8_t low = xt_pic_read(&emu->pic, dx_port_ax);
                uint8_t high = xt_pic_read(&emu->pic, dx_port_ax + 1);
                dx_in_value_ax = (high << 8) | low;
            } else if (dx_port_ax >= XT_PIT_BASE && dx_port_ax <= XT_PIT_BASE + 3) {
                /* Read PIT register (16-bit read) */
                uint8_t low = xt_pit_read(&emu->pit, dx_port_ax);
                uint8_t high = xt_pit_read(&emu->pit, dx_port_ax + 1);
                dx_in_value_ax = (high << 8) | low;
            } else if (dx_port_ax == XT_KEYBOARD) {
                /* Read keyboard controller data port (16-bit read) */
                uint8_t low = xt_keyboard_read(&emu->keyboard, dx_port_ax);
                uint8_t high = xt_keyboard_read(&emu->keyboard, dx_port_ax + 1);
                dx_in_value_ax = (high << 8) | low;
            } else if (dx_port_ax < 16) {
                /* Read DMA register (16-bit read) */
                uint8_t low = xt_dma_read(&emu->dma, dx_port_ax);
                uint8_t high = xt_dma_read(&emu->dma, dx_port_ax + 1);
                dx_in_value_ax = (high << 8) | low;
            } else {
                printf("IN from port DX=0x%04X (AX = 0x%04X)\n", dx_port_ax, dx_in_value_ax);
            }
            
            emu->cpu.ax = dx_in_value_ax;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 10;
            break;
            
        case 0xEE: /* OUT DX,AL */
            /* Output AL to port (DX) */
            uint16_t dx_out_port = emu->cpu.dx;
            uint8_t dx_out_value = emu->cpu.ax & 0xFF;
            
            /* Handle specific ports */
            if (dx_out_port == XT_PIC_MASTER || dx_out_port == XT_PIC_MASTER + 1) {
                /* Write to PIC register */
                xt_pic_write(&emu->pic, dx_out_port, dx_out_value);
            } else if (dx_out_port >= XT_PIT_BASE && dx_out_port <= XT_PIT_BASE + 3) {
                /* Write to PIT register */
                xt_pit_write(&emu->pit, dx_out_port, dx_out_value);
            } else if (dx_out_port == XT_KEYBOARD) {
                /* Write to keyboard controller data port */
                xt_keyboard_write(&emu->keyboard, dx_out_port, dx_out_value);
            } else if (dx_out_port < 16) {
                /* Write to DMA register */
                xt_dma_write(&emu->dma, dx_out_port, dx_out_value);
            } else {
                printf("OUT to port DX=0x%04X (value 0x%02X)\n", dx_out_port, dx_out_value);
            }
            
            emu->cpu.ip += 1;
            emu->cpu.cycles += 10;
            break;
            
        case 0xEF: /* OUT DX,AX */
            /* Output AX to port (DX) */
            uint16_t dx_out_port_ax = emu->cpu.dx;
            uint16_t dx_out_value_ax = emu->cpu.ax;
            
            /* Handle specific ports */
            if (dx_out_port_ax == XT_PIC_MASTER || dx_out_port_ax == XT_PIC_MASTER + 1) {
                /* Write to PIC register (16-bit write) */
                xt_pic_write(&emu->pic, dx_out_port_ax, dx_out_value_ax & 0xFF);
                xt_pic_write(&emu->pic, dx_out_port_ax + 1, (dx_out_value_ax >> 8) & 0xFF);
            } else if (dx_out_port_ax >= XT_PIT_BASE && dx_out_port_ax <= XT_PIT_BASE + 3) {
                /* Write to PIT register (16-bit write) */
                xt_pit_write(&emu->pit, dx_out_port_ax, dx_out_value_ax & 0xFF);
                xt_pit_write(&emu->pit, dx_out_port_ax + 1, (dx_out_value_ax >> 8) & 0xFF);
            } else if (dx_out_port_ax == XT_KEYBOARD) {
                /* Write to keyboard controller data port (16-bit write) */
                xt_keyboard_write(&emu->keyboard, dx_out_port_ax, dx_out_value_ax & 0xFF);
                xt_keyboard_write(&emu->keyboard, dx_out_port_ax + 1, (dx_out_value_ax >> 8) & 0xFF);
            } else if (dx_out_port_ax < 16) {
                /* Write to DMA register (16-bit write) */
                xt_dma_write(&emu->dma, dx_out_port_ax, dx_out_value_ax & 0xFF);
                xt_dma_write(&emu->dma, dx_out_port_ax + 1, (dx_out_value_ax >> 8) & 0xFF);
            } else {
                printf("OUT to port DX=0x%04X (value 0x%04X)\n", dx_out_port_ax, dx_out_value_ax);
            }
            
            emu->cpu.ip += 1;
            emu->cpu.cycles += 10;
            break;
            
        case 0x00: /* ADD AL,imm8 */
            /* Add immediate to AL */
            uint8_t imm8 = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            emu->cpu.ax += imm8;
            emu->cpu.ip += 2;
            emu->cpu.cycles += 4;
            break;
            
        case 0x04: /* ADD AL,imm8 */
            /* Add immediate to AL */
            uint8_t al_imm = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            emu->cpu.ax += al_imm;
            emu->cpu.ip += 2;
            emu->cpu.cycles += 4;
            break;
            
        case 0xB8: /* MOV AX,imm16 */
            /* Move immediate 16-bit to AX */
            uint16_t imm16 = xt_memory_read_word(&emu->memory, cs_ip + 1);
            emu->cpu.ax = imm16;
            emu->cpu.ip += 3;
            emu->cpu.cycles += 4;
            break;
            
        /* Arithmetic Instructions */
        case 0x14: /* ADD AL,imm8 */
            /* Add immediate to AL */
            uint8_t adc_imm = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            uint8_t al_old = emu->cpu.ax & 0xFF;
            uint8_t adc_cf = (emu->cpu.flags & 0x0001) ? 1 : 0;
            uint16_t result = al_old + adc_imm + adc_cf;
            emu->cpu.ax = (emu->cpu.ax & 0xFF00) | (result & 0xFF);
            emu->cpu.flags = (emu->cpu.flags & ~0x0001) | ((result & 0x100) ? 0x0001 : 0);
            emu->cpu.flags |= (result == 0) ? 0x0040 : 0;
            emu->cpu.ip += 2;
            emu->cpu.cycles += 4;
            break;
            
        case 0x1C: /* SBB AL,imm8 */
            /* Subtract with borrow immediate from AL */
            uint8_t sbb_imm = xt_memory_read_byte(&emu->memory, cs_ip + 1);
            uint8_t al_val = emu->cpu.ax & 0xFF;
            uint8_t sbb_cf = (emu->cpu.flags & 0x0001) ? 1 : 0;
            uint16_t sbb_result = al_val - sbb_imm - sbb_cf;
            emu->cpu.ax = (emu->cpu.ax & 0xFF00) | (sbb_result & 0xFF);
            emu->cpu.flags = (emu->cpu.flags & ~0x0001) | ((sbb_result & 0x100) ? 0x0001 : 0);
            emu->cpu.flags |= (sbb_result == 0) ? 0x0040 : 0;
            emu->cpu.ip += 2;
            emu->cpu.cycles += 4;
            break;
            
        case 0x27: /* DAA - Decimal Adjust AL */
            /* Decimal adjust AL after addition */
            uint8_t al = emu->cpu.ax & 0xFF;
            if ((emu->cpu.flags & 0x0004) || (al & 0x0F) > 9) {
                al += 6;
                emu->cpu.flags |= 0x0001;
            }
            if ((emu->cpu.flags & 0x0001) || (al & 0xF0) > 0x90) {
                al += 0x60;
                emu->cpu.flags |= 0x0001;
            }
            emu->cpu.ax = (emu->cpu.ax & 0xFF00) | (al & 0xFF);
            emu->cpu.flags |= (al == 0) ? 0x0040 : 0;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 4;
            break;
            
        case 0x2F: /* AAS - ASCII Adjust AL */
            /* ASCII adjust AL after subtraction */
            uint8_t aas_al = emu->cpu.ax & 0xFF;
            if ((emu->cpu.flags & 0x0004) || (aas_al & 0x0F) > 9) {
                aas_al -= 6;
                emu->cpu.flags |= 0x0001;
            }
            if ((emu->cpu.flags & 0x0001) || (aas_al & 0xF0) > 0x90) {
                aas_al -= 0x60;
                emu->cpu.flags |= 0x0001;
            }
            emu->cpu.ax = (emu->cpu.ax & 0xFF00) | (aas_al & 0xFF);
            emu->cpu.flags |= (aas_al == 0) ? 0x0040 : 0;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 4;
            break;
            
        /* Logical Instructions */
        case 0xF6: /* Various 8-bit operations */
            /* Decode based on mod/rm byte */
            uint8_t mod_rm = emu->memory.bios_rom[cs_ip + 1];
            uint8_t reg = (mod_rm >> 3) & 0x07;
            
            switch (reg) {
                case 2: /* NEG - Negate operand */
                    /* For now, negate AL */
                    uint8_t neg_val = ~(emu->cpu.ax & 0xFF) + 1;
                    emu->cpu.ax = (emu->cpu.ax & 0xFF00) | (neg_val & 0xFF);
                    emu->cpu.flags |= (neg_val == 0) ? 0x0040 : 0;
                    emu->cpu.flags |= (neg_val == 0x80) ? 0x0001 : 0;
                    break;
                case 4: /* MUL - Unsigned multiplication */
                    /* AL * operand -> AX */
                    uint8_t mul_val = emu->cpu.ax & 0xFF;
                    uint16_t mul_result = mul_val * mul_val;
                    emu->cpu.ax = mul_result;
                    emu->cpu.flags = (mul_result == 0) ? 0 : 0x0001;
                    break;
                case 5: /* IMUL - Signed multiplication */
                    /* AL * operand -> AX */
                    int8_t imul_val = emu->cpu.ax & 0xFF;
                    int16_t imul_result = imul_val * imul_val;
                    emu->cpu.ax = imul_result;
                    emu->cpu.flags = (imul_result == 0) ? 0 : 0x0001;
                    break;
                case 6: /* DIV - Unsigned division */
                    /* AX / operand -> AL, remainder -> AH */
                    uint8_t div_val = emu->cpu.ax & 0xFF;
                    if (div_val == 0) {
                        emu->cpu.flags |= 0x0001; /* Division by zero */
                    } else {
                        uint8_t al = emu->cpu.ax / div_val;
                        uint8_t ah = emu->cpu.ax % div_val;
                        emu->cpu.ax = (ah << 8) | al;
                    }
                    break;
                case 7: /* IDIV - Signed division */
                    /* AX / operand -> AL, remainder -> AH */
                    int8_t idiv_val = emu->cpu.ax & 0xFF;
                    if (idiv_val == 0) {
                        emu->cpu.flags |= 0x0001; /* Division by zero */
                    } else {
                        int8_t al = emu->cpu.ax / idiv_val;
                        int8_t ah = emu->cpu.ax % idiv_val;
                        emu->cpu.ax = (ah << 8) | (al & 0xFF);
                    }
                    break;
                case 0: /* TEST AL,imm8 */
                    uint8_t test_imm = xt_memory_read_byte(&emu->memory, cs_ip + 2);
                    uint8_t test_result = (emu->cpu.ax & 0xFF) & test_imm;
                    emu->cpu.flags = (test_result == 0) ? 0x0040 : 0;
                    emu->cpu.ip += 3;
                    emu->cpu.cycles += 5;
                    break;
            }
            emu->cpu.ip += 2;
            emu->cpu.cycles += 17;
            break;
            
        /* String Instructions */
        case 0xA4: /* MOVSB - Move byte string */
            /* Move byte from [DS:SI] to [ES:DI] */
            uint32_t src_addr = xt_memory_segment_to_linear(emu->cpu.ds, emu->cpu.si);
            uint32_t dst_addr = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            
            uint8_t movsb_val = xt_memory_read_byte(&emu->memory, src_addr);
            xt_memory_write_byte(&emu->memory, dst_addr, movsb_val);
            
            emu->cpu.si += (emu->cpu.flags & 0x0004) ? -1 : 1;
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -1 : 1;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 18;
            break;
            
        case 0xA5: /* MOVSW - Move word string */
            /* Move word from [DS:SI] to [ES:DI] */
            uint32_t movsw_src = xt_memory_segment_to_linear(emu->cpu.ds, emu->cpu.si);
            uint32_t movsw_dst = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            
            uint16_t movsw_val = xt_memory_read_word(&emu->memory, movsw_src);
            xt_memory_write_word(&emu->memory, movsw_dst, movsw_val);
            
            emu->cpu.si += (emu->cpu.flags & 0x0004) ? -2 : 2;
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -2 : 2;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 18;
            break;
            
        case 0xA6: /* CMPSB - Compare byte string */
            /* Compare [DS:SI] with [ES:DI] */
            uint32_t cmpsb_src = xt_memory_segment_to_linear(emu->cpu.ds, emu->cpu.si);
            uint32_t cmpsb_dst = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            
            uint8_t cmpsb_val1 = xt_memory_read_byte(&emu->memory, cmpsb_src);
            uint8_t cmpsb_val2 = xt_memory_read_byte(&emu->memory, cmpsb_dst);
            uint16_t cmpsb_result = cmpsb_val1 - cmpsb_val2;
            emu->cpu.flags = (cmpsb_result == 0) ? 0x0040 : 0;
            emu->cpu.flags |= (cmpsb_result & 0x100) ? 0x0001 : 0;
            emu->cpu.si += (emu->cpu.flags & 0x0004) ? -1 : 1;
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -1 : 1;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 22;
            break;
            
        case 0xA7: /* CMPSW - Compare word string */
            /* Compare [DS:SI] with [ES:DI] */
            uint32_t cmpsw_src = xt_memory_segment_to_linear(emu->cpu.ds, emu->cpu.si);
            uint32_t cmpsw_dst = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            
            uint16_t cmpsw_val1 = xt_memory_read_word(&emu->memory, cmpsw_src);
            uint16_t cmpsw_val2 = xt_memory_read_word(&emu->memory, cmpsw_dst);
            uint32_t cmpsw_result = cmpsw_val1 - cmpsw_val2;
            emu->cpu.flags = (cmpsw_result == 0) ? 0x0040 : 0;
            emu->cpu.flags |= (cmpsw_result & 0x10000) ? 0x0001 : 0;
            emu->cpu.si += (emu->cpu.flags & 0x0004) ? -2 : 2;
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -2 : 2;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 22;
            break;
            
        case 0xAC: /* LODSB - Load byte string */
            /* Load byte from [DS:SI] to AL */
            uint32_t lodsb_src = xt_memory_segment_to_linear(emu->cpu.ds, emu->cpu.si);
            uint8_t lodsb_val = xt_memory_read_byte(&emu->memory, lodsb_src);
            emu->cpu.ax = (emu->cpu.ax & 0xFF00) | lodsb_val;
            emu->cpu.si += (emu->cpu.flags & 0x0004) ? -1 : 1;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 12;
            break;
            
        case 0xAD: /* LODSW - Load word string */
            /* Load word from [DS:SI] to AX */
            uint32_t lodsw_src = xt_memory_segment_to_linear(emu->cpu.ds, emu->cpu.si);
            uint16_t lodsw_val = xt_memory_read_word(&emu->memory, lodsw_src);
            emu->cpu.ax = lodsw_val;
            emu->cpu.si += (emu->cpu.flags & 0x0004) ? -2 : 2;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 12;
            break;
            
        case 0xAA: /* STOSB - Store byte string */
            /* Store AL to [ES:DI] */
            uint32_t stosb_dst = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            xt_memory_write_byte(&emu->memory, stosb_dst, emu->cpu.ax & 0xFF);
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -1 : 1;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 10;
            break;
            
        case 0xAB: /* STOSW - Store word string */
            /* Store AX to [ES:DI] */
            uint32_t stosw_dst = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            xt_memory_write_word(&emu->memory, stosw_dst, emu->cpu.ax);
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -2 : 2;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 10;
            break;
            
        case 0xAE: /* SCASB - Scan byte string */
            /* Scan AL for byte at [ES:DI] */
            uint32_t scasb_src = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            uint8_t scasb_val = xt_memory_read_byte(&emu->memory, scasb_src);
            uint16_t scasb_result = (emu->cpu.ax & 0xFF) - scasb_val;
            emu->cpu.flags = (scasb_result == 0) ? 0x0040 : 0;
            emu->cpu.flags |= (scasb_result & 0x100) ? 0x0001 : 0;
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -1 : 1;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 15;
            break;
            
        case 0xAF: /* SCASW - Scan word string */
            /* Scan AX for word at [ES:DI] */
            uint32_t scasw_src = xt_memory_segment_to_linear(emu->cpu.es, emu->cpu.di);
            uint16_t scasw_val = xt_memory_read_word(&emu->memory, scasw_src);
            uint32_t scasw_result = emu->cpu.ax - scasw_val;
            emu->cpu.flags = (scasw_result == 0) ? 0x0040 : 0;
            emu->cpu.flags |= (scasw_result & 0x10000) ? 0x0001 : 0;
            emu->cpu.di += (emu->cpu.flags & 0x0004) ? -2 : 2;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 15;
            break;
            
        /* Flag Manipulation Instructions */
        case 0xF9: /* STC - Set Carry Flag */
            emu->cpu.flags |= 0x0001;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 2;
            break;
            
        case 0xF8: /* CLC - Clear Carry Flag */
            emu->cpu.flags &= ~0x0001;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 2;
            break;
            
        case 0xF5: /* CMC - Complement Carry Flag */
            emu->cpu.flags ^= 0x0001;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 2;
            break;
            
        case 0xFD: /* STD - Set Direction Flag */
            emu->cpu.flags |= 0x0004;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 2;
            break;
            
        case 0xFC: /* CLD - Clear Direction Flag */
            emu->cpu.flags &= ~0x0004;
            emu->cpu.ip += 1;
            emu->cpu.cycles += 2;
            break;
            
        case 0x9F: /* LAHF - Load AH from Flags */
            emu->cpu.bx = (emu->cpu.bx & 0xFF00) | (emu->cpu.flags & 0xFF);
            emu->cpu.ip += 1;
            emu->cpu.cycles += 4;
            break;
            
        case 0x9E: /* SAHF - Store AH to Flags */
            emu->cpu.flags = (emu->cpu.flags & 0xFF00) | (emu->cpu.bx & 0xFF);
            emu->cpu.ip += 1;
            emu->cpu.cycles += 4;
            break;
            
        /* Additional Control Flow Instructions */
        case 0xE0: /* LOOPNE - Loop if CX != 0 and ZF=0 */
            emu->cpu.cx--;
            if (emu->cpu.cx != 0 && !(emu->cpu.flags & 0x0040)) {
                int8_t loop_offset = emu->memory.bios_rom[cs_ip + 1];
                emu->cpu.ip += 2 + (int8_t)loop_offset;
            } else {
                emu->cpu.ip += 2;
            }
            emu->cpu.cycles += (emu->cpu.cx == 0) ? 5 : 17;
            break;
            
        case 0xE1: /* LOOPE - Loop if CX != 0 and ZF=1 */
            emu->cpu.cx--;
            if (emu->cpu.cx != 0 && (emu->cpu.flags & 0x0040)) {
                int8_t loop_offset = emu->memory.bios_rom[cs_ip + 1];
                emu->cpu.ip += 2 + (int8_t)loop_offset;
            } else {
                emu->cpu.ip += 2;
            }
            emu->cpu.cycles += (emu->cpu.cx == 0) ? 5 : 17;
            break;
            
        case 0xE3: /* JCXZ - Jump if CX = 0 */
            if (emu->cpu.cx == 0) {
                int8_t jcxz_offset = emu->memory.bios_rom[cs_ip + 1];
                emu->cpu.ip += 2 + (int8_t)jcxz_offset;
            } else {
                emu->cpu.ip += 2;
            }
            emu->cpu.cycles += 6;
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
    int instruction_count = 0;
    
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
                    xt_keyboard_handle_sdl_event(&emu->keyboard, &event);
                    /* Trigger keyboard interrupt if key is available */
                    xt_keyboard_trigger_irq(&emu->keyboard, &emu->pic);
                    break;
                case SDL_KEYUP:
                    /* Handle keyboard release */
                    xt_keyboard_handle_sdl_event(&emu->keyboard, &event);
                    /* Trigger keyboard interrupt if key is available */
                    xt_keyboard_trigger_irq(&emu->keyboard, &emu->pic);
                    break;
            }
        }
        
        /* Execute CPU instruction */
        xt_step(emu);
        
        /* Update PIT timer */
        xt_pit_update(&emu->pit, emu->cpu.cycles);
        
        /* Check for timer interrupt */
        if (emu->cpu.flags & 0x0002) { /* If interrupts enabled */
            xt_pit_trigger_irq0(&emu->pit, &emu->pic);
        }
        
        /* Debug output */
        if (instruction_count % 1000 == 0) {
            printf("CS:IP = %04X:%04X, AX = %04X, BX = %04X, CX = %04X, DX = %04X, SP = %04X\n",
                   emu->cpu.cs, emu->cpu.ip, emu->cpu.ax, emu->cpu.bx, emu->cpu.cx, emu->cpu.dx, emu->cpu.sp);
        }
        instruction_count++;
        
        /* Update display */
        if (emu->display.initialized) {
            /* Render text mode from video memory */
            xt_display_render_text_mode(&emu->display, &emu->memory, &emu->cpu);
            
            /* Update SDL display */
            xt_display_update(&emu->display);
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