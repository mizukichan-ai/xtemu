#include "xtemu.h"
#include "ibm_pc_font.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Convert CGA color to ARGB8888 */
static uint32_t cga_to_argb(uint8_t cga_color) {
    uint8_t r, g, b;
    
    switch (cga_color & 0x0F) {
        case XT_COLOR_BLACK:     r = 0x00; g = 0x00; b = 0x00; break;
        case XT_COLOR_BLUE:      r = 0x00; g = 0x00; b = 0xAA; break;
        case XT_COLOR_GREEN:     r = 0x00; g = 0xAA; b = 0x00; break;
        case XT_COLOR_CYAN:      r = 0x00; g = 0xAA; b = 0xAA; break;
        case XT_COLOR_RED:       r = 0xAA; g = 0x00; b = 0x00; break;
        case XT_COLOR_MAGENTA:   r = 0xAA; g = 0x00; b = 0xAA; break;
        case XT_COLOR_BROWN:     r = 0xAA; g = 0x55; b = 0x00; break;
        case XT_COLOR_LIGHT_GRAY: r = 0xAA; g = 0xAA; b = 0xAA; break;
        case XT_COLOR_DARK_GRAY:  r = 0x55; g = 0x55; b = 0x55; break;
        case XT_COLOR_LIGHT_BLUE: r = 0x55; g = 0x55; b = 0xFF; break;
        case XT_COLOR_LIGHT_GREEN: r = 0x55; g = 0xFF; b = 0x55; break;
        case XT_COLOR_LIGHT_CYAN: r = 0x55; g = 0xFF; b = 0xFF; break;
        case XT_COLOR_LIGHT_RED:  r = 0xFF; g = 0x55; b = 0x55; break;
        case XT_COLOR_LIGHT_MAGENTA: r = 0xFF; g = 0x55; b = 0xFF; break;
        case XT_COLOR_YELLOW:    r = 0xFF; g = 0xFF; b = 0x55; break;
        case XT_COLOR_WHITE:     r = 0xFF; g = 0xFF; b = 0xFF; break;
        default:                 r = 0x00; g = 0x00; b = 0x00; break;
    }
    
    /* Brightness bit */
    if (cga_color & 0x10) {
        r = (r * 2) | 0x01;
        g = (g * 2) | 0x01;
        b = (b * 2) | 0x01;
    }
    
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void xt_display_init(xt_display_t *display) {
    if (display->initialized) {
        return;
    }
    
    /* Initialize video mode */
    display->video_mode = XT_VIDEO_MODE_TEXT_80x25;
    display->text_width = XT_TEXT_WIDTH;
    display->text_height = XT_TEXT_HEIGHT;
    display->char_width = XT_VGA_WIDTH / XT_TEXT_WIDTH;
    display->char_height = XT_VGA_HEIGHT / XT_TEXT_HEIGHT;
    
    /* Initialize cursor */
    display->cursor_x = 0;
    display->cursor_y = 0;
    display->cursor_visible = true;
    display->cursor_shape = 0x0B; /* Underline cursor */
    display->current_attribute = 0x07; /* Default white text on black background */
    
    /* Initialize color palette */
    for (int i = 0; i < 16; i++) {
        display->colors[i] = cga_to_argb(i);
    }
    
    display->initialized = true;
}

void xt_display_cleanup(xt_display_t *display) {
    if (!display->initialized) {
        return;
    }
    
    if (display->texture) {
        SDL_DestroyTexture(display->texture);
        display->texture = NULL;
    }
    
    if (display->renderer) {
        SDL_DestroyRenderer(display->renderer);
        display->renderer = NULL;
    }
    
    if (display->window) {
        SDL_DestroyWindow(display->window);
        display->window = NULL;
    }
    
    if (display->framebuffer) {
        free(display->framebuffer);
        display->framebuffer = NULL;
    }
    
    display->initialized = false;
}

void xt_display_update(xt_display_t *display) {
    if (!display->initialized || !display->texture) {
        return;
    }
    
    SDL_UpdateTexture(display->texture, NULL, 
                      display->framebuffer, 
                      XT_VGA_WIDTH * 4);
    
    SDL_RenderClear(display->renderer);
    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}

void xt_display_render_text_mode(xt_display_t *display, xt_memory_t *memory, xt_cpu_t *cpu) {
    if (!display->initialized || !display->framebuffer) {
        return;
    }
    
    uint32_t *pixels = (uint32_t *)display->framebuffer;
    uint16_t video_offset = 0;
    
    /* Clear framebuffer to black */
    memset(display->framebuffer, 0, XT_VGA_WIDTH * XT_VGA_HEIGHT * 4);
    
    /* Render text from video memory */
    for (uint16_t y = 0; y < display->text_height; y++) {
        for (uint16_t x = 0; x < display->text_width; x++) {
            uint16_t char_pos = y * display->text_width + x;
            
            /* Get character and attribute from video memory */
            uint8_t character = memory->video_ram[char_pos * 2];
            uint8_t attribute = memory->video_ram[char_pos * 2 + 1];
            
            /* Extract colors from attribute byte */
            uint8_t foreground = attribute & 0x0F;
            uint8_t background = (attribute >> 4) & 0x0F;
            uint32_t fg_color = display->colors[foreground];
            uint32_t bg_color = display->colors[background];
            
            /* Get character bitmap */
            const uint8_t *char_bitmap = ibm_pc_get_char_bitmap(character);
            
            /* Render character */
            for (uint8_t cy = 0; cy < 8; cy++) {
                for (uint8_t cx = 0; cx < 8; cx++) {
                    uint16_t pixel_x = x * display->char_width + cx;
                    uint16_t pixel_y = y * display->char_height + cy;
                    
                    if (pixel_x < XT_VGA_WIDTH && pixel_y < XT_VGA_HEIGHT) {
                        uint32_t pixel_index = pixel_y * XT_VGA_WIDTH + pixel_x;
                        
                        if (char_bitmap[cy] & (0x80 >> cx)) {
                            pixels[pixel_index] = fg_color;
                        } else {
                            pixels[pixel_index] = bg_color;
                        }
                    }
                }
            }
        }
    }
    
    /* Draw cursor */
    if (display->cursor_visible) {
        uint16_t cursor_x = display->cursor_x * display->char_width;
        uint16_t cursor_y = display->cursor_y * display->char_height + display->char_height - 2;
        
        for (uint16_t x = 0; x < display->char_width; x++) {
            for (uint16_t y = 0; y < 2; y++) {
                uint16_t pixel_x = cursor_x + x;
                uint16_t pixel_y = cursor_y + y;
                
                if (pixel_x < XT_VGA_WIDTH && pixel_y < XT_VGA_HEIGHT) {
                    uint32_t pixel_index = pixel_y * XT_VGA_WIDTH + pixel_x;
                    pixels[pixel_index] ^= 0x00FFFFFF; /* Invert colors for cursor */
                }
            }
        }
    }
}

void xt_display_set_video_mode(xt_display_t *display, uint8_t mode) {
    if (!display->initialized) {
        return;
    }
    
    display->video_mode = mode;
    
    switch (mode) {
        case XT_VIDEO_MODE_TEXT_80x25:
            display->text_width = 80;
            display->text_height = 25;
            display->char_width = XT_VGA_WIDTH / 80;
            display->char_height = XT_VGA_HEIGHT / 25;
            break;
            
        case XT_VIDEO_MODE_TEXT_40x25:
            display->text_width = 40;
            display->text_height = 25;
            display->char_width = XT_VGA_WIDTH / 40;
            display->char_height = XT_VGA_HEIGHT / 25;
            break;
            
        default:
            /* Unsupported mode, default to 80x25 text */
            display->text_width = 80;
            display->text_height = 25;
            display->char_width = XT_VGA_WIDTH / 80;
            display->char_height = XT_VGA_HEIGHT / 25;
            break;
    }
}

uint8_t xt_display_get_video_mode(xt_display_t *display) {
    return display->video_mode;
}

void xt_display_update_cursor(xt_display_t *display, uint16_t x, uint16_t y) {
    if (!display->initialized) {
        return;
    }
    
    display->cursor_x = x;
    display->cursor_y = y;
}

/* Scroll display up by one line */
void xt_display_scroll_up(xt_display_t *display, xt_memory_t *memory) {
    if (!display || !memory || display->video_mode != XT_VIDEO_MODE_TEXT_80x25) {
        return;
    }
    
    /* Copy video memory up by one line */
    for (int y = 1; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            uint16_t src_offset = (y * 80 + x) * 2;
            uint16_t dst_offset = ((y - 1) * 80 + x) * 2;
            
            memory->video_ram[dst_offset] = memory->video_ram[src_offset];
            memory->video_ram[dst_offset + 1] = memory->video_ram[src_offset + 1];
        }
    }
    
    /* Clear the last line */
    for (int x = 0; x < 80; x++) {
        uint16_t offset = (24 * 80 + x) * 2;
        memory->video_ram[offset] = ' ';
        memory->video_ram[offset + 1] = 0x07; /* Default color */
    }
    
    /* Update cursor if it was on the bottom line */
    if (display->cursor_y == 24) {
        display->cursor_y = 23;
    }
}

/* Scroll display down by one line */
void xt_display_scroll_down(xt_display_t *display, xt_memory_t *memory) {
    if (!display || !memory || display->video_mode != XT_VIDEO_MODE_TEXT_80x25) {
        return;
    }
    
    /* Copy video memory down by one line */
    for (int y = 24; y > 0; y--) {
        for (int x = 0; x < 80; x++) {
            uint16_t src_offset = ((y - 1) * 80 + x) * 2;
            uint16_t dst_offset = (y * 80 + x) * 2;
            
            memory->video_ram[dst_offset] = memory->video_ram[src_offset];
            memory->video_ram[dst_offset + 1] = memory->video_ram[src_offset + 1];
        }
    }
    
    /* Clear the first line */
    for (int x = 0; x < 80; x++) {
        uint16_t offset = x * 2;
        memory->video_ram[offset] = ' ';
        memory->video_ram[offset + 1] = 0x07; /* Default color */
    }
    
    /* Update cursor if it was on the top line */
    if (display->cursor_y == 0) {
        display->cursor_y = 1;
    }
}

/* Insert a character at current cursor position */
void xt_display_insert_char(xt_display_t *display, xt_memory_t *memory, char character) {
    if (!display || !memory || display->video_mode != XT_VIDEO_MODE_TEXT_80x25) {
        return;
    }
    
    uint16_t offset = (display->cursor_y * 80 + display->cursor_x) * 2;
    
    /* Store character and attribute */
    memory->video_ram[offset] = character;
    memory->video_ram[offset + 1] = display->current_attribute;
    
    /* Move cursor right */
    display->cursor_x++;
    if (display->cursor_x >= 80) {
        display->cursor_x = 0;
        display->cursor_y++;
        
        /* If we reached the bottom, scroll up */
        if (display->cursor_y >= 25) {
            xt_display_scroll_up(display, memory);
            display->cursor_y = 24;
        }
    }
}

/* Move cursor to specified position */
void xt_display_set_cursor(xt_display_t *display, uint8_t x, uint8_t y) {
    if (!display) {
        return;
    }
    
    if (display->video_mode == XT_VIDEO_MODE_TEXT_80x25) {
        display->cursor_x = x % 80;
        display->cursor_y = y % 25;
    } else if (display->video_mode == XT_VIDEO_MODE_TEXT_40x25) {
        display->cursor_x = x % 40;
        display->cursor_y = y % 25;
    }
}

/* Set current text attribute */
void xt_display_set_attribute(xt_display_t *display, uint8_t attribute) {
    if (!display) {
        return;
    }
    
    display->current_attribute = attribute;
}