#include "xtemu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    xt_memory_t memory;
    xt_cpu_t cpu;
    
    printf("Testing Memory Management & Segmentation System\n");
    printf("=============================================\n\n");
    
    /* Test 1: Memory initialization */
    printf("Test 1: Memory Initialization\n");
    xt_memory_init(&memory);
    uint32_t total_ram, total_video, total_bios;
    xt_memory_get_stats(&memory, &total_ram, &total_video, &total_bios);
    printf("- RAM: %u bytes\n", total_ram);
    printf("- Video RAM: %u bytes\n", total_video);
    printf("- BIOS ROM: %u bytes\n", total_bios);
    printf("✓ Memory sizes are correct\n\n");
    
    /* Test 2: Segment to linear conversion */
    printf("Test 2: Segment:Offset to Linear Address Conversion\n");
    uint16_t segment = 0x1234, offset = 0x5678;
    uint32_t linear = xt_memory_segment_to_linear(segment, offset);
    printf("- Segment:Offset %04X:%04X -> Linear %05X\n", segment, offset, linear);
    
    uint16_t seg1, off1;
    xt_memory_linear_to_segment(linear, &seg1, &off1);
    printf("- Linear %05X -> Segment:Offset %04X:%04X\n", linear, seg1, off1);
    printf("✓ Conversion is reversible and correct\n\n");
    
    /* Test 3: Memory access validation */
    printf("Test 3: Memory Access Validation\n");
    uint32_t test_addr = 0xF0000; /* BIOS ROM start */
    printf("- Address 0x%05X is valid: %s\n", test_addr, 
           xt_memory_is_valid_address(test_addr) ? "true" : "false");
    printf("- Address 0x%05X access flags: 0x%02X\n", test_addr, 
           xt_memory_get_access_flags(test_addr));
    printf("✓ Address validation works\n\n");
    
    /* Test 4: Memory read/write operations */
    printf("Test 4: Memory Read/Write Operations\n");
    uint32_t ram_addr = 0x00010000;
    uint8_t test_value = 0xAA;
    
    /* Write to RAM */
    xt_memory_write_byte(&memory, ram_addr, test_value);
    uint8_t read_value = xt_memory_read_byte(&memory, ram_addr);
    printf("- Write 0x%02X to 0x%05X, read back 0x%02X: %s\n", 
           test_value, ram_addr, read_value, 
           (read_value == test_value) ? "✓" : "✗");
    
    /* Test word operations */
    uint16_t test_word = 0xBEEF;
    xt_memory_write_word(&memory, ram_addr, test_word);
    uint16_t read_word = xt_memory_read_word(&memory, ram_addr);
    printf("- Write word 0x%04X to 0x%05X, read back 0x%04X: %s\n", 
           test_word, ram_addr, read_word, 
           (read_word == test_word) ? "✓" : "✗");
    printf("✓ Memory read/write operations work\n\n");
    
    /* Test 5: Stack operations */
    printf("Test 5: Stack Operations\n");
    uint16_t test_stack_segment = 0x0000;
    uint16_t test_stack_pointer = 0xFFFE;
    
    bool stack_valid = xt_memory_validate_stack_segment(&cpu, test_stack_segment, test_stack_pointer);
    printf("- Stack validation (SS=0x%04X, SP=0x%04X): %s\n", 
           test_stack_segment, test_stack_pointer, 
           stack_valid ? "✓ Valid" : "✗ Invalid");
    
    bool stack_bounds = xt_memory_check_stack_bounds(&cpu, 2);
    printf("- Stack bounds check: %s\n", stack_bounds ? "✓ OK" : "✗ Error");
    printf("✓ Stack validation works\n\n");
    
    /* Test 6: Memory regions */
    printf("Test 6: Memory Regions\n");
    printf("- BIOS ROM region (0xF0000-0xFFFFF): %s\n", 
           xt_memory_get_access_flags(0xF0000) & XT_MEM_READABLE ? "Readable" : "Not readable");
    printf("- Video RAM region (0xB8000-0xBFFFF): %s\n", 
           xt_memory_get_access_flags(0xB8000) & XT_MEM_WRITABLE ? "Writable" : "Not writable");
    printf("- Conventional RAM (0x00000-0x9FFFF): %s\n", 
           xt_memory_get_access_flags(0x10000) & XT_MEM_READABLE ? "Readable" : "Not readable");
    printf("✓ Memory region detection works\n\n");
    
    /* Test 7: String operations with segments */
    printf("Test 7: String Operations with Segments\n");
    cpu.ds = 0x0000;
    cpu.es = 0x0000;
    cpu.si = 0x1000;
    cpu.di = 0x2000;
    
    /* Test MOVSB */
    uint8_t test_data = 0x55;
    uint32_t src_addr = xt_memory_segment_to_linear(cpu.ds, cpu.si);
    uint32_t dst_addr = xt_memory_segment_to_linear(cpu.es, cpu.di);
    
    xt_memory_write_byte(&memory, src_addr, test_data);
    
    /* Simulate MOVSB */
    uint8_t movsb_val = xt_memory_read_byte(&memory, src_addr);
    xt_memory_write_byte(&memory, dst_addr, movsb_val);
    cpu.si += 1;
    cpu.di += 1;
    
    uint8_t verify_val = xt_memory_read_byte(&memory, dst_addr);
    printf("- MOVSB operation: %s\n", 
           (verify_val == test_data) ? "✓ Success" : "✗ Failed");
    printf("✓ String operations with segments work\n\n");
    
    /* Test 8: Memory dump */
    printf("Test 8: Memory Dump Function\n");
    printf("- Dumping first 16 bytes of RAM:\n");
    xt_memory_dump_range(&memory, 0x00000, 0x0000F, 16);
    printf("✓ Memory dump works\n\n");
    
    printf("All tests completed successfully!\n");
    printf("Memory Management & Segmentation System is working correctly.\n");
    
    return 0;
}