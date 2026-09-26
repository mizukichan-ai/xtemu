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

/* Memory access flags */
#define XT_MEM_READABLE    0x01
#define XT_MEM_WRITABLE    0x02
#define XT_MEM_EXECUTABLE  0x04

/* Hardware registers */
#define XT_PIC_MASTER 0x20
#define XT_PIC_SLAVE  0xA0
#define XT_PIT_BASE   0x40
#define XT_DMA_BASE   0x00
#define XT_KEYBOARD   0x60

/* PIC register offsets */
#define XT_PIC_ICW1       0x20
#define XT_PIC_OCW2       0x20
#define XT_PIC_OCW3       0x20
#define XT_PIC_ICW2       0x21
#define XT_PIC_ICW3       0x21
#define XT_PIC_ICW4       0x21
#define XT_PIC_ISR        0x20
#define XT_PIC_IMR        0x21
#define XT_PIC_IRR        0x20

#define XT_PIC_SLAVE_ICW1 0xA0
#define XT_PIC_SLAVE_OCW2 0xA0
#define XT_PIC_SLAVE_OCW3 0xA0
#define XT_PIC_SLAVE_ICW2 0xA1
#define XT_PIC_SLAVE_ICW3 0xA1
#define XT_PIC_SLAVE_ICW4 0xA1
#define XT_PIC_SLAVE_ISR   0xA0
#define XT_PIC_SLAVE_IMR   0xA1
#define XT_PIC_SLAVE_IRR   0xA0

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

/* PIC (Programmable Interrupt Controller) structure */
struct xt_pic {
    /* Master PIC */
    uint8_t icw1;
    uint8_t icw2;
    uint8_t icw3;
    uint8_t icw4;
    uint8_t imr;
    uint8_t irr;
    uint8_t isr;
    uint8_t auto_eoi;
    uint8_t read_isr;
    
    /* Slave PIC */
    uint8_t slave_icw1;
    uint8_t slave_icw2;
    uint8_t slave_icw3;
    uint8_t slave_icw4;
    uint8_t slave_imr;
    uint8_t slave_irr;
    uint8_t slave_isr;
    uint8_t slave_auto_eoi;
    uint8_t slave_read_isr;
    
    /* State */
    bool initialized;
    bool slave_initialized;
    uint8_t cascade_vector;
};

/* Main emulator state */
typedef struct {
    xt_cpu_t cpu;
    xt_memory_t memory;
    xt_display_t display;
    xt_keyboard_t keyboard;
    struct xt_pic pic;
    bool     running;
    bool     debug_mode;
} xt_emulator_t;

/* Function prototypes */
int xt_init(xt_emulator_t *emu);
void xt_cleanup(xt_emulator_t *emu);
void xt_reset(xt_emulator_t *emu);
void xt_step(xt_emulator_t *emu);
void xt_run(xt_emulator_t *emu);

/* PIC (Programmable Interrupt Controller) functions */
void xt_pic_init(struct xt_pic *pic);
void xt_pic_write(struct xt_pic *pic, uint16_t port, uint8_t value);
uint8_t xt_pic_read(struct xt_pic *pic, uint16_t port);
void xt_pic_trigger_irq(struct xt_pic *pic, uint8_t irq);
int xt_pic_get_highest_irq(struct xt_pic *pic);
uint8_t xt_pic_get_interrupt_vector(struct xt_pic *pic, uint8_t irq);
void xt_pic_send_eoi(struct xt_pic *pic, uint8_t irq);
void xt_pic_get_status(struct xt_pic *pic, uint8_t *initialized, uint8_t *slave_initialized, 
                       uint8_t *master_imr, uint8_t *slave_imr);

/* Memory management functions */
void xt_memory_init(xt_memory_t *memory);
int xt_memory_load_bios(xt_memory_t *memory, const char *filename);
uint32_t xt_memory_segment_to_linear(uint16_t segment, uint16_t offset);
void xt_memory_linear_to_segment(uint32_t linear, uint16_t *segment, uint16_t *offset);
bool xt_memory_is_valid_address(uint32_t address);
uint8_t xt_memory_get_access_flags(uint32_t address);
uint8_t xt_memory_read_byte(xt_memory_t *memory, uint32_t address);
void xt_memory_write_byte(xt_memory_t *memory, uint32_t address, uint8_t value);
uint16_t xt_memory_read_word(xt_memory_t *memory, uint32_t address);
void xt_memory_write_word(xt_memory_t *memory, uint32_t address, uint16_t value);
uint32_t xt_memory_read_dword(xt_memory_t *memory, uint32_t address);
void xt_memory_write_dword(xt_memory_t *memory, uint32_t address, uint32_t value);
void xt_memory_dump_range(xt_memory_t *memory, uint32_t start, uint32_t end, uint16_t bytes_per_line);
void xt_memory_get_stats(xt_memory_t *memory, uint32_t *total_ram, uint32_t *total_video, uint32_t *total_bios);
bool xt_memory_validate_stack_segment(xt_cpu_t *cpu, uint16_t stack_segment, uint16_t stack_pointer);
bool xt_memory_check_stack_bounds(xt_cpu_t *cpu, uint16_t stack_size);

#endif /* XTEMU_H */