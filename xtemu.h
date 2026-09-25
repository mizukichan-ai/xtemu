#ifndef XTEMU_H
#define XTEMU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

/* CPU definitions */
#define XT_CPU_FREQ 4770000    /* 4.77MHz */
#define XT_CPU_TURBO_FREQ 7160000 /* 7.16MHz */

/* Memory definitions */
#define XT_RAM_SIZE 640 * 1024  /* 640KB */

/* Hardware registers */
#define XT_PIC_MASTER 0x20
#define XT_PIC_SLAVE  0xA0
#define XT_PIT_BASE   0x40
#define XT_DMA_BASE   0x00
#define XT_KEYBOARD   0x60

/* Video modes */
#define XT_VGA_WIDTH  640
#define XT_VGA_HEIGHT 480

/* Forward declarations */
typedef struct xt_cpu xt_cpu_t;
typedef struct xt_memory xt_memory_t;
typedef struct xt_display xt_display_t;
typedef struct xt_keyboard xt_keyboard_t;

/* CPU structure */
struct xt_cpu {
    uint16_t ax, bx, cx, dx;
    uint16_t si, di, bp, sp;
    uint16_t ip, cs, ds, es, ss;
    uint8_t  flags;
    uint32_t cycles;
    bool     turbo_mode;
};

/* Memory structure */
struct xt_memory {
    uint8_t ram[XT_RAM_SIZE];
    uint8_t bios_rom[65536];
    uint8_t video_ram[32768];
};

/* Display structure */
struct xt_display {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint8_t *framebuffer;
    bool     initialized;
};

/* Keyboard structure */
struct xt_keyboard {
    SDL_Scancode key_map[256];
    uint8_t     keyboard_buffer[16];
    uint8_t     buffer_head;
    uint8_t     buffer_tail;
};

/* Main emulator state */
typedef struct {
    xt_cpu_t cpu;
    xt_memory_t memory;
    xt_display_t display;
    xt_keyboard_t keyboard;
    bool     running;
    bool     debug_mode;
} xt_emulator_t;

/* Function prototypes */
int xt_init(xt_emulator_t *emu);
void xt_cleanup(xt_emulator_t *emu);
void xt_reset(xt_emulator_t *emu);
void xt_step(xt_emulator_t *emu);
void xt_run(xt_emulator_t *emu);

#endif /* XTEMU_H */