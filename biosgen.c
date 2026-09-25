#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IBM XT BIOS entry point */
#define BIOS_ENTRY_POINT 0xFFF0

/* Basic BIOS ROM with proper entry point */
void create_bios_rom(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to create BIOS ROM file");
        exit(1);
    }
    
    // Allocate 64KB for BIOS ROM
    uint8_t *bios = malloc(65536);
    if (!bios) {
        perror("Failed to allocate memory for BIOS ROM");
        fclose(file);
        exit(1);
    }
    
    // Initialize with zeros
    memset(bios, 0x00, 65536);
    
    // Set up entry point (jump to start of BIOS code)
    bios[0xFFFE] = 0xE9; // JMP opcode
    bios[0xFFFF] = 0x00; // Low byte of jump target
    bios[0x0000] = 0xF0; // High byte of jump target (F000:0000)
    
    // Basic BIOS initialization code at F000:0000
    // Power-on self test and memory check
    uint16_t bios_offset = 0xF000;
    
    // JMP to main BIOS routine
    bios[bios_offset + 0x0000] = 0xE9; // JMP
    bios[bios_offset + 0x0001] = 0x3C; // Low byte (F000:003C)
    bios[bios_offset + 0x0002] = 0xF0; // High byte
    
    // NOP padding until we implement actual BIOS code
    for (int i = 0x0003; i < 0x0040; i++) {
        bios[bios_offset + i] = 0x90; // NOP
    }
    
    // Basic system information
    bios[bios_offset + 0x0040] = 0x55; // Signature byte
    bios[bios_offset + 0x0041] = 0xAA; // Signature byte
    
    // Video mode setup (EGA/VGA)
    bios[bios_offset + 0x0049] = 0x12; // Video mode: 80x25 text, 16 colors
    
    // Memory size detection (640KB)
    bios[bios_offset + 0x004C] = 0x00; // Low byte of memory size
    bios[bios_offset + 0x004D] = 0xA0; // High byte (0xA000 = 640KB)
    
    // Equipment list
    bios[bios_offset + 0x010] = 0xF8; // Diskette, math coprocessor, 32KB RAM
    bios[bios_offset + 0x011] = 0x01; // Original PC equipment list
    
    // Diskette parameters
    bios[bios_offset + 0x0E] = 0x1C; // 40 tracks, 9 sectors, 2 sides
    bios[bios_offset + 0x1E] = 0x4C; // Gap length, data length, etc.
    
    // Write BIOS ROM to file
    fwrite(bios, 1, 65536, file);
    
    fclose(file);
    free(bios);
    
    printf("BIOS ROM created: %s\n", filename);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <output_file>\n", argv[0]);
        return 1;
    }
    
    create_bios_rom(argv[1]);
    return 0;
}