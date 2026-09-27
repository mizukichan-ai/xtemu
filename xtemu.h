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

/* Video text mode constants */
#define XT_TEXT_WIDTH    80
#define XT_TEXT_HEIGHT   25
#define XT_TEXT_CHARS    (XT_TEXT_WIDTH * XT_TEXT_HEIGHT)

/* Video memory addresses */
#define XT_VIDEO_RAM_START    0xB8000
#define XT_VIDEO_RAM_SIZE     32768  /* 32KB for text mode */

/* Video modes */
#define XT_VIDEO_MODE_TEXT_80x25    0x03
#define XT_VIDEO_MODE_TEXT_40x25    0x02
#define XT_VIDEO_MODE_CGA_320x200   0x04
#define XT_VIDEO_MODE_CGA_640x350   0x0D
#define XT_VIDEO_MODE_VGA_640x480   0x12

/* Color palette (CGA) */
#define XT_COLOR_BLACK     0x00
#define XT_COLOR_BLUE      0x01
#define XT_COLOR_GREEN     0x02
#define XT_COLOR_CYAN      0x03
#define XT_COLOR_RED       0x04
#define XT_COLOR_MAGENTA   0x05
#define XT_COLOR_BROWN     0x06
#define XT_COLOR_LIGHT_GRAY 0x07
#define XT_COLOR_DARK_GRAY  0x08
#define XT_COLOR_LIGHT_BLUE 0x09
#define XT_COLOR_LIGHT_GREEN 0x0A
#define XT_COLOR_LIGHT_CYAN 0x0B
#define XT_COLOR_LIGHT_RED  0x0C
#define XT_COLOR_LIGHT_MAGENTA 0x0D
#define XT_COLOR_YELLOW     0x0E
#define XT_COLOR_WHITE      0x0F

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
    
    /* Video mode information */
    uint8_t  video_mode;
    uint16_t text_width;
    uint16_t text_height;
    uint16_t char_width;
    uint16_t char_height;
    
    /* Text mode state */
    uint16_t cursor_x;
    uint16_t cursor_y;
    bool     cursor_visible;
    uint8_t  cursor_shape;
    
    /* Color palette */
    uint32_t colors[16];
};

/* Keyboard structure */
struct xt_keyboard {
    SDL_Scancode key_map[256];
    uint8_t     keyboard_buffer[16];
    uint8_t     buffer_head;
    uint8_t     buffer_tail;
    uint8_t     status_port;
    uint8_t     data_port;
    bool        keyboard_enabled;
    uint8_t     scan_code_set;
};

/* DMA structure */
struct xt_dma_channel {
    uint8_t mode;           /* Mode register */
    uint8_t address;        /* Address register (low byte) */
    uint8_t address_high;   /* Address register (high byte) */
    uint8_t count;          /* Count register (low byte) */
    uint8_t count_high;     /* Count register (high byte) */
    uint8_t page;           /* Page register */
    bool    enabled;        /* Channel enabled */
    bool    auto_init;      /* Auto-initialize mode */
    uint8_t direction;      /* Direction: 0=device->mem, 1=mem->device */
    uint8_t transfer_type;  /* Transfer type */
};

struct xt_dma {
    /* DMA channels (4 channels: 0-3) */
    struct xt_dma_channel channels[4];
    
    /* Control registers */
    uint8_t command;           /* Command register */
    uint8_t status;            /* Status register */
    uint8_t request;           /* Request register */
    uint8_t single_mask;       /* Single channel mask */
    uint8_t all_mask;          /* All channels mask */
    
    /* State */
    bool initialized;
    uint8_t cascade_channel;   /* Cascade channel (channel 4) */
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

/* PIT (Programmable Interval Timer) structure */
struct xt_pit {
    /* Timer counters (3 channels) */
    struct {
        uint16_t counter;    /* Current counter value */
        uint16_t latch;      /* Latched counter value */
        uint8_t mode;        /* Mode of operation */
        uint8_t command;     /* Command register */
        bool bcd;           /* BCD vs binary mode */
        bool read_back;     /* Read back mode */
        uint8_t status;     /* Status flags */
    } channels[3];
    
    /* Control register */
    uint8_t control;
    
    /* State */
    bool initialized;
    uint32_t last_tick;   /* Last timer tick */
    uint32_t tick_count;  /* Total tick count */
    bool timer_running;   /* Timer running state */
};

/* Main emulator state */
typedef struct {
    xt_cpu_t cpu;
    xt_memory_t memory;
    xt_display_t display;
    xt_keyboard_t keyboard;
    struct xt_pic pic;
    struct xt_pit pit;
    struct xt_dma dma;
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

/* PIT (Programmable Interval Timer) functions */
void xt_pit_init(struct xt_pit *pit);
void xt_pit_write(struct xt_pit *pit, uint16_t port, uint8_t value);
uint8_t xt_pit_read(struct xt_pit *pit, uint16_t port);
void xt_pit_update(struct xt_pit *pit, uint32_t cycles);
void xt_pit_trigger_irq0(struct xt_pit *pit, struct xt_pic *pic);

/* DMA Controller functions */
void xt_dma_init(struct xt_dma *dma);
void xt_dma_write(struct xt_dma *dma, uint16_t port, uint8_t value);
uint8_t xt_dma_read(struct xt_dma *dma, uint16_t port);
void xt_dma_trigger_transfer(struct xt_dma *dma, uint8_t channel);
uint8_t xt_dma_get_status(struct xt_dma *dma);
void xt_dma_set_mask(struct xt_dma *dma, uint8_t channel, bool masked);
void xt_dma_clear_mask(struct xt_dma *dma, uint8_t channel);

/* Keyboard Controller functions */
void xt_keyboard_init(struct xt_keyboard *keyboard);
void xt_keyboard_write(struct xt_keyboard *keyboard, uint16_t port, uint8_t value);
uint8_t xt_keyboard_read(struct xt_keyboard *keyboard, uint16_t port);
void xt_keyboard_handle_sdl_event(struct xt_keyboard *keyboard, SDL_Event *event);
bool xt_keyboard_has_key(struct xt_keyboard *keyboard);
uint8_t xt_keyboard_read_scancode(struct xt_keyboard *keyboard);
void xt_keyboard_trigger_irq(struct xt_keyboard *keyboard, struct xt_pic *pic);

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

/* Video display functions */
void xt_display_init(xt_display_t *display);
void xt_display_cleanup(xt_display_t *display);
void xt_display_update(xt_display_t *display);
void xt_display_render_text_mode(xt_display_t *display, xt_memory_t *memory, xt_cpu_t *cpu);
void xt_display_set_video_mode(xt_display_t *display, uint8_t mode);
uint8_t xt_display_get_video_mode(xt_display_t *display);
void xt_display_update_cursor(xt_display_t *display, uint16_t x, uint16_t y);

#endif /* XTEMU_H */