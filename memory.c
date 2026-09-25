#include "xtemu.h"
#include <stdio.h>
#include <string.h>

/* Memory region definitions */
#define XT_BIOS_ROM_START    0xF0000
#define XT_BIOS_ROM_END      0xFFFFF
#define XT_VIDEO_RAM_START   0xB8000
#define XT_VIDEO_RAM_END     0xBFFFF
#define XT_SYSTEM_AREA_START 0x00000
#define XT_SYSTEM_AREA_END   0x003FF

/* Memory access flags */
#define XT_MEM_READABLE    0x01
#define XT_MEM_WRITABLE    0x02
#define XT_MEM_EXECUTABLE  0x04

/* Memory region structure */
struct xt_memory_region {
    uint32_t start;
    uint32_t end;
    uint8_t  flags;
    const char *name;
};

/* Memory regions table */
static const struct xt_memory_region memory_regions[] = {
    {XT_BIOS_ROM_START, XT_BIOS_ROM_END, XT_MEM_READABLE | XT_MEM_EXECUTABLE, "BIOS ROM"},
    {XT_VIDEO_RAM_START, XT_VIDEO_RAM_END, XT_MEM_READABLE | XT_MEM_WRITABLE, "Video RAM"},
    {XT_SYSTEM_AREA_START, XT_SYSTEM_AREA_END, XT_MEM_READABLE | XT_MEM_WRITABLE, "System Area"},
    {0x00000, 0x9FFFF, XT_MEM_READABLE | XT_MEM_WRITABLE | XT_MEM_EXECUTABLE, "Conventional RAM"},
    {0xA0000, 0xBFFFF, XT_MEM_READABLE | XT_MEM_WRITABLE, "Video RAM (A0000-BFFFF)"},
    {0xC0000, 0xEFFFF, XT_MEM_READABLE | XT_MEM_EXECUTABLE, "Expansion ROM"},
    {0xF0000, 0xFFFFF, XT_MEM_READABLE | XT_MEM_EXECUTABLE, "BIOS ROM"}
};

/* Initialize memory system */
void xt_memory_init(xt_memory_t *memory) {
    /* Clear all memory */
    memset(memory->ram, 0, sizeof(memory->ram));
    memset(memory->video_ram, 0, sizeof(memory->video_ram));
    memset(memory->bios_rom, 0, sizeof(memory->bios_rom));
    
    printf("Memory system initialized:\n");
    printf("- RAM: %lu bytes\n", (unsigned long)sizeof(memory->ram));
    printf("- Video RAM: %lu bytes\n", (unsigned long)sizeof(memory->video_ram));
    printf("- BIOS ROM: %lu bytes\n", (unsigned long)sizeof(memory->bios_rom));
}

/* Load BIOS ROM from file */
int xt_memory_load_bios(xt_memory_t *memory, const char *filename) {
    FILE *bios_file = fopen(filename, "rb");
    if (!bios_file) {
        printf("Warning: %s not found, using NOP BIOS\n", filename);
        memset(memory->bios_rom, 0x90, sizeof(memory->bios_rom)); // NOPs for now
        return -1;
    }
    
    size_t bytes_read = fread(memory->bios_rom, 1, sizeof(memory->bios_rom), bios_file);
    fclose(bios_file);
    
    printf("BIOS ROM loaded: %zu bytes from %s\n", bytes_read, filename);
    return 0;
}

/* Convert segment:offset to linear address */
uint32_t xt_memory_segment_to_linear(uint16_t segment, uint16_t offset) {
    return ((uint32_t)segment << 4) + offset;
}

/* Convert linear address to segment:offset */
void xt_memory_linear_to_segment(uint32_t linear, uint16_t *segment, uint16_t *offset) {
    *segment = (uint16_t)(linear >> 4);
    *offset = (uint16_t)(linear & 0x0F);
}

/* Check if memory address is valid */
bool xt_memory_is_valid_address(uint32_t address) {
    return address < 0x100000; // 1MB address space
}

/* Check if address is within a specific region */
bool xt_memory_is_in_region(uint32_t address, uint32_t start, uint32_t end) {
    return address >= start && address <= end;
}

/* Get memory access flags for address */
uint8_t xt_memory_get_access_flags(uint32_t address) {
    for (size_t i = 0; i < sizeof(memory_regions) / sizeof(memory_regions[0]); i++) {
        if (xt_memory_is_in_region(address, memory_regions[i].start, memory_regions[i].end)) {
            return memory_regions[i].flags;
        }
    }
    return 0; /* Invalid address */
}

/* Read byte from memory with proper bounds checking */
uint8_t xt_memory_read_byte(xt_memory_t *memory, uint32_t address) {
    if (!xt_memory_is_valid_address(address)) {
        printf("Memory read error: invalid address 0x%05X\n", address);
        return 0xFF;
    }
    
    uint8_t access_flags = xt_memory_get_access_flags(address);
    if (!(access_flags & XT_MEM_READABLE)) {
        printf("Memory read error: address 0x%05X not readable\n", address);
        return 0xFF;
    }
    
    /* Map address to appropriate memory space */
    if (xt_memory_is_in_region(address, XT_BIOS_ROM_START, XT_BIOS_ROM_END)) {
        return memory->bios_rom[address - XT_BIOS_ROM_START];
    } else if (xt_memory_is_in_region(address, XT_VIDEO_RAM_START, XT_VIDEO_RAM_END)) {
        return memory->video_ram[address - XT_VIDEO_RAM_START];
    } else {
        return memory->ram[address];
    }
}

/* Write byte to memory with proper bounds checking */
void xt_memory_write_byte(xt_memory_t *memory, uint32_t address, uint8_t value) {
    if (!xt_memory_is_valid_address(address)) {
        printf("Memory write error: invalid address 0x%05X\n", address);
        return;
    }
    
    uint8_t access_flags = xt_memory_get_access_flags(address);
    if (!(access_flags & XT_MEM_WRITABLE)) {
        printf("Memory write error: address 0x%05X not writable\n", address);
        return;
    }
    
    /* Map address to appropriate memory space */
    if (xt_memory_is_in_region(address, XT_VIDEO_RAM_START, XT_VIDEO_RAM_END)) {
        memory->video_ram[address - XT_VIDEO_RAM_START] = value;
    } else {
        memory->ram[address] = value;
    }
}

/* Read word from memory */
uint16_t xt_memory_read_word(xt_memory_t *memory, uint32_t address) {
    uint8_t low = xt_memory_read_byte(memory, address);
    uint8_t high = xt_memory_read_byte(memory, address + 1);
    return (high << 8) | low;
}

/* Write word to memory */
void xt_memory_write_word(xt_memory_t *memory, uint32_t address, uint16_t value) {
    xt_memory_write_byte(memory, address, value & 0xFF);
    xt_memory_write_byte(memory, address + 1, (value >> 8) & 0xFF);
}

/* Read dword from memory */
uint32_t xt_memory_read_dword(xt_memory_t *memory, uint32_t address) {
    uint16_t low = xt_memory_read_word(memory, address);
    uint16_t high = xt_memory_read_word(memory, address + 2);
    return (high << 16) | low;
}

/* Write dword to memory */
void xt_memory_write_dword(xt_memory_t *memory, uint32_t address, uint32_t value) {
    xt_memory_write_word(memory, address, value & 0xFFFF);
    xt_memory_write_word(memory, address + 2, (value >> 16) & 0xFFFF);
}

/* Dump memory range to console */
void xt_memory_dump_range(xt_memory_t *memory, uint32_t start, uint32_t end, uint16_t bytes_per_line) {
    printf("Memory dump from 0x%05X to 0x%05X:\n", start, end);
    
    for (uint32_t addr = start; addr <= end; addr += bytes_per_line) {
        printf("%05X: ", addr);
        for (uint16_t i = 0; i < bytes_per_line && addr + i <= end; i++) {
            printf("%02X ", xt_memory_read_byte(memory, addr + i));
        }
        printf("\n");
    }
}

/* Get memory statistics */
void xt_memory_get_stats(xt_memory_t *memory, uint32_t *total_ram, uint32_t *total_video, uint32_t *total_bios) {
    *total_ram = sizeof(memory->ram);
    *total_video = sizeof(memory->video_ram);
    *total_bios = sizeof(memory->bios_rom);
}

/* Validate stack pointer for segment */
bool xt_memory_validate_stack_segment(xt_cpu_t *cpu, uint16_t stack_segment, uint16_t stack_pointer) {
    (void)cpu; /* Unused parameter - kept for API compatibility */
    uint32_t stack_base = xt_memory_segment_to_linear(stack_segment, 0);
    uint32_t stack_top = xt_memory_segment_to_linear(stack_segment, 0xFFFF);
    uint32_t stack_addr = xt_memory_segment_to_linear(stack_segment, stack_pointer);
    
    /* Stack should be within segment bounds */
    if (stack_addr < stack_base || stack_addr > stack_top) {
        printf("Stack pointer out of bounds: SS=0x%04X, SP=0x%04X\n", stack_segment, stack_pointer);
        return false;
    }
    
    /* Stack should be in writable memory */
    uint8_t access_flags = xt_memory_get_access_flags(stack_addr);
    if (!(access_flags & XT_MEM_WRITABLE)) {
        printf("Stack not in writable memory: SS=0x%04X, SP=0x%04X\n", stack_segment, stack_pointer);
        return false;
    }
    
    return true;
}

/* Check for stack overflow/underflow */
bool xt_memory_check_stack_bounds(xt_cpu_t *cpu, uint16_t stack_size) {
    uint32_t stack_addr = xt_memory_segment_to_linear(cpu->ss, cpu->sp);
    uint32_t stack_top = xt_memory_segment_to_linear(cpu->ss, 0xFFFF);
    
    /* Check for stack overflow (stack growing too large) */
    if (stack_addr + stack_size > stack_top) {
        printf("Stack overflow detected: SS=0x%04X, SP=0x%04X\n", cpu->ss, cpu->sp);
        return false;
    }
    
    /* Check for stack underflow (stack pointer too small) */
    if (cpu->sp < stack_size) {
        printf("Stack underflow detected: SS=0x%04X, SP=0x%04X\n", cpu->ss, cpu->sp);
        return false;
    }
    
    return true;
}