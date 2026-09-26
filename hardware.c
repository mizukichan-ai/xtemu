#include "xtemu.h"
#include <stdio.h>
#include <string.h>

/* PIC (Programmable Interrupt Controller) definitions */

/* PIC interrupt vectors */
static const uint8_t pic_interrupt_vectors[] = {
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* IRQ 0-7 */
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77   /* IRQ 8-15 */
};

/* Initialize PIC system */
void xt_pic_init(struct xt_pic *pic) {
    /* Clear all PIC registers */
    memset(pic, 0, sizeof(struct xt_pic));
    
    printf("PIC system initialized\n");
}

/* Write to PIC register */
void xt_pic_write(struct xt_pic *pic, uint16_t port, uint8_t value) {
    bool is_slave = (port >= XT_PIC_SLAVE_ICW1);
    struct xt_pic *target = is_slave ? pic : pic;
    
    if (port == XT_PIC_ICW1 || port == XT_PIC_SLAVE_ICW1) {
        /* ICW1 - Initialization Control Word 1 */
        if (value & 0x10) {
            /* Initialization sequence started */
            target->icw1 = value;
            
            if (!is_slave) {
                printf("Master PIC initialization started\n");
            } else {
                printf("Slave PIC initialization started\n");
            }
        }
    }
    else if (port == XT_PIC_ICW2 || port == XT_PIC_SLAVE_ICW2) {
        /* ICW2 - Interrupt Vector Offset */
        if (target->icw1 & 0x02) {
            target->icw2 = value;
            if (!is_slave) {
                printf("Master PIC vector offset set to 0x%02X\n", value);
            } else {
                printf("Slave PIC vector offset set to 0x%02X\n", value);
            }
        }
    }
    else if (port == XT_PIC_ICW3 || port == XT_PIC_SLAVE_ICW3) {
        /* ICW3 - Cascade/Slave Mode */
        if (target->icw1 & 0x04) {
            target->icw3 = value;
            if (!is_slave) {
                printf("Master PIC cascade bits set to 0x%02X\n", value);
            } else {
                printf("Slave PIC cascade ID set to %d\n", value);
            }
        }
    }
    else if (port == XT_PIC_ICW4 || port == XT_PIC_SLAVE_ICW4) {
        /* ICW4 - Mode Control */
        if (target->icw1 & 0x01) {
            target->icw4 = value;
            if (!is_slave) {
                pic->initialized = true;
                printf("Master PIC initialized with mode 0x%02X\n", value);
            } else {
                pic->slave_initialized = true;
                printf("Slave PIC initialized with mode 0x%02X\n", value);
            }
        }
    }
    else if (port == XT_PIC_IMR || port == XT_PIC_SLAVE_IMR) {
        /* IMR - Interrupt Mask Register */
        if (is_slave) {
            pic->slave_imr = value;
            printf("Slave PIC interrupt mask set to 0x%02X\n", value);
        } else {
            pic->imr = value;
            printf("Master PIC interrupt mask set to 0x%02X\n", value);
        }
    }
    else if (port == XT_PIC_OCW2 || port == XT_PIC_SLAVE_OCW2) {
        /* OCW2 - Operation Control Word 2 (EOI, priority rotation) */
        uint8_t irq = value & 0x07;
        uint8_t eoi = (value & 0x20) >> 5;
        uint8_t rotate = (value & 0x80) >> 7;
        
        if (eoi) {
            /* End of Interrupt */
            if (is_slave) {
                pic->slave_isr &= ~(1 << irq);
                printf("Slave EOI for IRQ %d\n", irq);
            } else {
                pic->isr &= ~(1 << irq);
                printf("Master EOI for IRQ %d\n", irq);
            }
        }
        
        if (rotate) {
            /* Priority rotation */
            if (is_slave) {
                printf("Slave PIC priority rotation enabled\n");
            } else {
                printf("Master PIC priority rotation enabled\n");
            }
        }
    }
    else if (port == XT_PIC_OCW3 || port == XT_PIC_SLAVE_OCW3) {
        /* OCW3 - Operation Control Word 3 (Special mask, poll) */
        uint8_t poll = (value & 0x04) >> 2;
        uint8_t special_mask = (value & 0x40) >> 6;
        
        if (poll) {
            /* Poll command */
            uint8_t highest_irq = 0;
            uint8_t status = 0;
            
            if (is_slave) {
                status = pic->slave_irr & ~pic->slave_imr;
                if (status) {
                    highest_irq = __builtin_ctz(status) + 8;
                    printf("Slave poll: IRQ %d pending\n", highest_irq);
                }
            } else {
                status = pic->irr & ~pic->imr;
                if (status) {
                    highest_irq = __builtin_ctz(status);
                    printf("Master poll: IRQ %d pending\n", highest_irq);
                }
            }
        }
        
        if (special_mask) {
            /* Special mask mode */
            if (is_slave) {
                printf("Slave PIC special mask mode enabled\n");
            } else {
                printf("Master PIC special mask mode enabled\n");
            }
        }
    }
}

/* Read from PIC register */
uint8_t xt_pic_read(struct xt_pic *pic, uint16_t port) {
    bool is_slave = (port >= XT_PIC_SLAVE_ICW1);
    
    if (port == XT_PIC_ISR || port == XT_PIC_SLAVE_ISR) {
        /* Read ISR (Interrupt Service Register) */
        if (is_slave) {
            return pic->slave_isr;
        } else {
            return pic->isr;
        }
    }
    else if (port == XT_PIC_IMR || port == XT_PIC_SLAVE_IMR) {
        /* Read IMR (Interrupt Mask Register) */
        if (is_slave) {
            return pic->slave_imr;
        } else {
            return pic->imr;
        }
    }
    else if (port == XT_PIC_IRR || port == XT_PIC_SLAVE_IRR) {
        /* Read IRR (Interrupt Request Register) */
        if (is_slave) {
            return pic->slave_irr;
        } else {
            return pic->irr;
        }
    }
    else if (port == XT_PIC_OCW3 || port == XT_PIC_SLAVE_OCW3) {
        /* OCW3 poll result */
        if (is_slave) {
            uint8_t status = pic->slave_irr & ~pic->slave_imr;
            return status ? 0x80 : 0x00;  // Non-zero if interrupt pending
        } else {
            uint8_t status = pic->irr & ~pic->imr;
            return status ? 0x80 : 0x00;  // Non-zero if interrupt pending
        }
    }
    
    return 0xFF;  /* Invalid register */
}

/* Trigger interrupt request */
void xt_pic_trigger_irq(struct xt_pic *pic, uint8_t irq) {
    if (irq < 16) {
        bool is_slave = (irq >= 8);
        struct xt_pic *target = is_slave ? pic : pic;
        uint8_t local_irq = is_slave ? (irq - 8) : irq;
        
        if (local_irq < 8) {
            /* Check if interrupt is masked */
            uint8_t imr = is_slave ? pic->slave_imr : pic->imr;
            if (!(imr & (1 << local_irq))) {
                target->irr |= (1 << local_irq);
                printf("IRQ %d requested (local IRQ %d)\n", irq, local_irq);
            }
        }
    }
}

/* Get highest priority interrupt */
int xt_pic_get_highest_irq(struct xt_pic *pic) {
    /* Check master PIC */
    uint8_t master_irr = pic->irr & ~pic->imr;
    if (master_irr) {
        return __builtin_ctz(master_irr);
    }
    
    /* Check slave PIC */
    uint8_t slave_irr = pic->slave_irr & ~pic->slave_imr;
    if (slave_irr) {
        return 8 + __builtin_ctz(slave_irr);
    }
    
    return -1;  /* No interrupts pending */
}

/* Get interrupt vector for IRQ */
uint8_t xt_pic_get_interrupt_vector(struct xt_pic *pic, uint8_t irq) {
    if (irq < 8) {
        return pic->icw2 + irq;
    } else if (irq < 16) {
        return pic->slave_icw2 + (irq - 8);
    }
    return 0xFF;
}

/* Send EOI (End of Interrupt) */
void xt_pic_send_eoi(struct xt_pic *pic, uint8_t irq) {
    if (irq < 8) {
        /* Master PIC EOI */
        pic->isr &= ~(1 << irq);
        if (!(pic->icw4 & 0x02)) {  /* Not auto-EOI mode */
            /* Send EOI to master */
            xt_pic_write(pic, XT_PIC_OCW2, 0x20);
        }
    } else if (irq < 16) {
        /* Slave PIC EOI */
        pic->slave_isr &= ~(1 << (irq - 8));
        if (!(pic->slave_icw4 & 0x02)) {  /* Not auto-EOI mode */
            /* Send EOI to slave first */
            xt_pic_write(pic, XT_PIC_SLAVE_OCW2, 0x20);
            /* Then send EOI to master */
            xt_pic_write(pic, XT_PIC_OCW2, 0x20 | (pic->icw3 & 0x02));
        }
    }
}

/* Get PIC status */
void xt_pic_get_status(struct xt_pic *pic, uint8_t *initialized, uint8_t *slave_initialized, 
                       uint8_t *master_imr, uint8_t *slave_imr) {
    *initialized = pic->initialized ? 1 : 0;
    *slave_initialized = pic->slave_initialized ? 1 : 0;
    *master_imr = pic->imr;
    *slave_imr = pic->slave_imr;
}

/* Keyboard Controller (8042) implementation */

/* Initialize keyboard controller */
void xt_keyboard_init(struct xt_keyboard *keyboard) {
    /* Clear keyboard state */
    memset(keyboard, 0, sizeof(struct xt_keyboard));
    
    /* Initialize keyboard buffer */
    keyboard->buffer_head = 0;
    keyboard->buffer_tail = 0;
    keyboard->keyboard_enabled = true;
    keyboard->scan_code_set = 1; /* Default to scan code set 1 */
    
    /* Initialize status and data ports */
    keyboard->status_port = 0x00;
    keyboard->data_port = 0x00;
    
    /* Initialize key mapping (PC keyboard layout) */
    memset(keyboard->key_map, 0, sizeof(keyboard->key_map));
    
    printf("Keyboard controller initialized\n");
}

/* Write to keyboard controller register */
void xt_keyboard_write(struct xt_keyboard *keyboard, uint16_t port, uint8_t value) {
    if (port == 0x60) { /* Data port */
        keyboard->data_port = value;
        
        /* Handle commands */
        switch (value) {
            case 0xED: /* Read keyboard mode */
                /* Set status to indicate response available */
                keyboard->status_port |= 0x01; /* Output buffer full */
                break;
                
            case 0xF0: /* Set scan code set */
                /* Next byte will specify the scan code set */
                keyboard->scan_code_set = 0xFF; /* Waiting for set number */
                break;
                
            case 0xF4: /* Enable keyboard */
                keyboard->keyboard_enabled = true;
                printf("Keyboard enabled\n");
                break;
                
            case 0xF5: /* Disable keyboard */
                keyboard->keyboard_enabled = false;
                printf("Keyboard disabled\n");
                break;
                
            default:
                printf("Keyboard command: 0x%02X\n", value);
                break;
        }
    } else if (port == 0x61) { /* System control port */
        /* Bit 0 controls PC speaker */
        /* Bit  controls keyboard clock */
        /* Bit 2 controls keyboard data */
        printf("System control port write: 0x%02X\n", value);
    }
}

/* Read from keyboard controller register */
uint8_t xt_keyboard_read(struct xt_keyboard *keyboard, uint16_t port) {
    if (port == 0x60) { /* Data port */
        /* Clear output buffer full flag */
        keyboard->status_port &= ~0x01;
        return keyboard->data_port;
    } else if (port == 0x61) { /* System control port */
        /* Return system control port status */
        return keyboard->status_port;
    }
    
    return 0xFF; /* Invalid port */
}

/* Handle SDL keyboard events */
void xt_keyboard_handle_sdl_event(struct xt_keyboard *keyboard, SDL_Event *event) {
    if (!keyboard->keyboard_enabled) {
        return;
    }
    
    uint8_t scancode = 0;
    
    switch (event->type) {
        case SDL_KEYDOWN:
            /* Map SDL scancode to XT scan code */
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE: scancode = 0x01; break;
                case SDL_SCANCODE_1: scancode = 0x02; break;
                case SDL_SCANCODE_2: scancode = 0x03; break;
                case SDL_SCANCODE_3: scancode = 0x04; break;
                case SDL_SCANCODE_4: scancode = 0x05; break;
                case SDL_SCANCODE_5: scancode = 0x06; break;
                case SDL_SCANCODE_6: scancode = 0x07; break;
                case SDL_SCANCODE_7: scancode = 0x08; break;
                case SDL_SCANCODE_8: scancode = 0x09; break;
                case SDL_SCANCODE_9: scancode = 0x0A; break;
                case SDL_SCANCODE_0: scancode = 0x0B; break;
                case SDL_SCANCODE_MINUS: scancode = 0x0C; break;
                case SDL_SCANCODE_EQUALS: scancode = 0x0D; break;
                case SDL_SCANCODE_BACKSPACE: scancode = 0x0E; break;
                case SDL_SCANCODE_TAB: scancode = 0x0F; break;
                case SDL_SCANCODE_Q: scancode = 0x10; break;
                case SDL_SCANCODE_W: scancode = 0x11; break;
                case SDL_SCANCODE_E: scancode = 0x12; break;
                case SDL_SCANCODE_R: scancode = 0x13; break;
                case SDL_SCANCODE_T: scancode = 0x14; break;
                case SDL_SCANCODE_Y: scancode = 0x15; break;
                case SDL_SCANCODE_U: scancode = 0x16; break;
                case SDL_SCANCODE_I: scancode = 0x17; break;
                case SDL_SCANCODE_O: scancode = 0x18; break;
                case SDL_SCANCODE_P: scancode = 0x19; break;
                case SDL_SCANCODE_LEFTBRACKET: scancode = 0x1A; break;
                case SDL_SCANCODE_RIGHTBRACKET: scancode = 0x1B; break;
                case SDL_SCANCODE_RETURN: scancode = 0x1C; break;
                case SDL_SCANCODE_LCTRL: scancode = 0x1D; break;
                case SDL_SCANCODE_A: scancode = 0x1E; break;
                case SDL_SCANCODE_S: scancode = 0x1F; break;
                case SDL_SCANCODE_D: scancode = 0x20; break;
                case SDL_SCANCODE_F: scancode = 0x21; break;
                case SDL_SCANCODE_G: scancode = 0x22; break;
                case SDL_SCANCODE_H: scancode = 0x23; break;
                case SDL_SCANCODE_J: scancode = 0x24; break;
                case SDL_SCANCODE_K: scancode = 0x25; break;
                case SDL_SCANCODE_L: scancode = 0x26; break;
                case SDL_SCANCODE_SEMICOLON: scancode = 0x27; break;
                case SDL_SCANCODE_APOSTROPHE: scancode = 0x28; break;
                case SDL_SCANCODE_GRAVE: scancode = 0x29; break;
                case SDL_SCANCODE_LSHIFT: scancode = 0x2A; break;
                case SDL_SCANCODE_BACKSLASH: scancode = 0x2B; break;
                case SDL_SCANCODE_Z: scancode = 0x2C; break;
                case SDL_SCANCODE_X: scancode = 0x2D; break;
                case SDL_SCANCODE_C: scancode = 0x2E; break;
                case SDL_SCANCODE_V: scancode = 0x2F; break;
                case SDL_SCANCODE_B: scancode = 0x30; break;
                case SDL_SCANCODE_N: scancode = 0x31; break;
                case SDL_SCANCODE_M: scancode = 0x32; break;
                case SDL_SCANCODE_COMMA: scancode = 0x33; break;
                case SDL_SCANCODE_PERIOD: scancode = 0x34; break;
                case SDL_SCANCODE_SLASH: scancode = 0x35; break;
                case SDL_SCANCODE_RSHIFT: scancode = 0x36; break;
                case SDL_SCANCODE_KP_MULTIPLY: scancode = 0x37; break;
                case SDL_SCANCODE_LALT: scancode = 0x38; break;
                case SDL_SCANCODE_SPACE: scancode = 0x39; break;
                case SDL_SCANCODE_CAPSLOCK: scancode = 0x3A; break;
                case SDL_SCANCODE_F1: scancode = 0x3B; break;
                case SDL_SCANCODE_F2: scancode = 0x3C; break;
                case SDL_SCANCODE_F3: scancode = 0x3D; break;
                case SDL_SCANCODE_F4: scancode = 0x3E; break;
                case SDL_SCANCODE_F5: scancode = 0x3F; break;
                case SDL_SCANCODE_F6: scancode = 0x40; break;
                case SDL_SCANCODE_F7: scancode = 0x41; break;
                case SDL_SCANCODE_F8: scancode = 0x42; break;
                case SDL_SCANCODE_F9: scancode = 0x43; break;
                case SDL_SCANCODE_F10: scancode = 0x44; break;
                case SDL_SCANCODE_NUMLOCKCLEAR: scancode = 0x45; break;
                case SDL_SCANCODE_SCROLLLOCK: scancode = 0x46; break;
                case SDL_SCANCODE_KP_7: scancode = 0x47; break;
                case SDL_SCANCODE_KP_8: scancode = 0x48; break;
                case SDL_SCANCODE_KP_9: scancode = 0x49; break;
                case SDL_SCANCODE_KP_MINUS: scancode = 0x4A; break;
                case SDL_SCANCODE_KP_4: scancode = 0x4B; break;
                case SDL_SCANCODE_KP_5: scancode = 0x4C; break;
                case SDL_SCANCODE_KP_6: scancode = 0x4D; break;
                case SDL_SCANCODE_KP_PLUS: scancode = 0x4E; break;
                case SDL_SCANCODE_KP_1: scancode = 0x4F; break;
                case SDL_SCANCODE_KP_2: scancode = 0x50; break;
                case SDL_SCANCODE_KP_3: scancode = 0x51; break;
                case SDL_SCANCODE_KP_0: scancode = 0x52; break;
                case SDL_SCANCODE_KP_PERIOD: scancode = 0x53; break;
                case SDL_SCANCODE_F11: scancode = 0x57; break;
                case SDL_SCANCODE_F12: scancode = 0x58; break;
                default:
                    scancode = 0x00; /* Unknown key */
                    break;
            }
            
            /* Add scan code to buffer if not full */
            if (keyboard->buffer_head != ((keyboard->buffer_tail + 15) & 0x0F)) {
                keyboard->keyboard_buffer[keyboard->buffer_head] = scancode;
                keyboard->buffer_head = (keyboard->buffer_head + 1) & 0x0F;
                printf("Key pressed: scan code 0x%02X\n", scancode);
            }
            break;
            
        case SDL_KEYUP:
            /* Handle key release (set bit 7 for make/break codes) */
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE: scancode = 0x81; break;
                case SDL_SCANCODE_1: scancode = 0x82; break;
                case SDL_SCANCODE_2: scancode = 0x83; break;
                case SDL_SCANCODE_3: scancode = 0x84; break;
                case SDL_SCANCODE_4: scancode = 0x85; break;
                case SDL_SCANCODE_5: scancode = 0x86; break;
                case SDL_SCANCODE_6: scancode = 0x87; break;
                case SDL_SCANCODE_7: scancode = 0x88; break;
                case SDL_SCANCODE_8: scancode = 0x89; break;
                case SDL_SCANCODE_9: scancode = 0x8A; break;
                case SDL_SCANCODE_0: scancode = 0x8B; break;
                case SDL_SCANCODE_MINUS: scancode = 0x8C; break;
                case SDL_SCANCODE_EQUALS: scancode = 0x8D; break;
                case SDL_SCANCODE_BACKSPACE: scancode = 0x8E; break;
                case SDL_SCANCODE_TAB: scancode = 0x8F; break;
                case SDL_SCANCODE_Q: scancode = 0x90; break;
                case SDL_SCANCODE_W: scancode = 0x91; break;
                case SDL_SCANCODE_E: scancode = 0x92; break;
                case SDL_SCANCODE_R: scancode = 0x93; break;
                case SDL_SCANCODE_T: scancode = 0x94; break;
                case SDL_SCANCODE_Y: scancode = 0x95; break;
                case SDL_SCANCODE_U: scancode = 0x96; break;
                case SDL_SCANCODE_I: scancode = 0x97; break;
                case SDL_SCANCODE_O: scancode = 0x98; break;
                case SDL_SCANCODE_P: scancode = 0x99; break;
                case SDL_SCANCODE_LEFTBRACKET: scancode = 0x9A; break;
                case SDL_SCANCODE_RIGHTBRACKET: scancode = 0x9B; break;
                case SDL_SCANCODE_RETURN: scancode = 0x9C; break;
                case SDL_SCANCODE_LCTRL: scancode = 0x9D; break;
                case SDL_SCANCODE_A: scancode = 0x9E; break;
                case SDL_SCANCODE_S: scancode = 0x9F; break;
                case SDL_SCANCODE_D: scancode = 0xA0; break;
                case SDL_SCANCODE_F: scancode = 0xA1; break;
                case SDL_SCANCODE_G: scancode = 0xA2; break;
                case SDL_SCANCODE_H: scancode = 0xA3; break;
                case SDL_SCANCODE_J: scancode = 0xA4; break;
                case SDL_SCANCODE_K: scancode = 0xA5; break;
                case SDL_SCANCODE_L: scancode = 0xA6; break;
                case SDL_SCANCODE_SEMICOLON: scancode = 0xA7; break;
                case SDL_SCANCODE_APOSTROPHE: scancode = 0xA8; break;
                case SDL_SCANCODE_GRAVE: scancode = 0xA9; break;
                case SDL_SCANCODE_LSHIFT: scancode = 0xAA; break;
                case SDL_SCANCODE_BACKSLASH: scancode = 0xAB; break;
                case SDL_SCANCODE_Z: scancode = 0xAC; break;
                case SDL_SCANCODE_X: scancode = 0xAD; break;
                case SDL_SCANCODE_C: scancode = 0xAE; break;
                case SDL_SCANCODE_V: scancode = 0xAF; break;
                case SDL_SCANCODE_B: scancode = 0xB0; break;
                case SDL_SCANCODE_N: scancode = 0xB1; break;
                case SDL_SCANCODE_M: scancode = 0xB2; break;
                case SDL_SCANCODE_COMMA: scancode = 0xB3; break;
                case SDL_SCANCODE_PERIOD: scancode = 0xB4; break;
                case SDL_SCANCODE_SLASH: scancode = 0xB5; break;
                case SDL_SCANCODE_RSHIFT: scancode = 0xB6; break;
                case SDL_SCANCODE_KP_MULTIPLY: scancode = 0xB7; break;
                case SDL_SCANCODE_LALT: scancode = 0xB8; break;
                case SDL_SCANCODE_SPACE: scancode = 0xB9; break;
                case SDL_SCANCODE_CAPSLOCK: scancode = 0xBA; break;
                case SDL_SCANCODE_F1: scancode = 0xBB; break;
                case SDL_SCANCODE_F2: scancode = 0xBC; break;
                case SDL_SCANCODE_F3: scancode = 0xBD; break;
                case SDL_SCANCODE_F4: scancode = 0xBE; break;
                case SDL_SCANCODE_F5: scancode = 0xBF; break;
                case SDL_SCANCODE_F6: scancode = 0xC0; break;
                case SDL_SCANCODE_F7: scancode = 0xC1; break;
                case SDL_SCANCODE_F8: scancode = 0xC2; break;
                case SDL_SCANCODE_F9: scancode = 0xC3; break;
                case SDL_SCANCODE_F10: scancode = 0xC4; break;
                case SDL_SCANCODE_NUMLOCKCLEAR: scancode = 0xC5; break;
                case SDL_SCANCODE_SCROLLLOCK: scancode = 0xC6; break;
                case SDL_SCANCODE_KP_7: scancode = 0xC7; break;
                case SDL_SCANCODE_KP_8: scancode = 0xC8; break;
                case SDL_SCANCODE_KP_9: scancode = 0xC9; break;
                case SDL_SCANCODE_KP_MINUS: scancode = 0xCA; break;
                case SDL_SCANCODE_KP_4: scancode = 0xCB; break;
                case SDL_SCANCODE_KP_5: scancode = 0xCC; break;
                case SDL_SCANCODE_KP_6: scancode = 0xCD; break;
                case SDL_SCANCODE_KP_PLUS: scancode = 0xCE; break;
                case SDL_SCANCODE_KP_1: scancode = 0xCF; break;
                case SDL_SCANCODE_KP_2: scancode = 0xD0; break;
                case SDL_SCANCODE_KP_3: scancode = 0xD1; break;
                case SDL_SCANCODE_KP_0: scancode = 0xD2; break;
                case SDL_SCANCODE_KP_PERIOD: scancode = 0xD3; break;
                case SDL_SCANCODE_F11: scancode = 0xD7; break;
                case SDL_SCANCODE_F12: scancode = 0xD8; break;
                default:
                    scancode = 0x00; /* Unknown key */
                    break;
            }
            
            /* Add break code to buffer if not full */
            if (keyboard->buffer_head != ((keyboard->buffer_tail + 15) & 0x0F)) {
                keyboard->keyboard_buffer[keyboard->buffer_head] = scancode;
                keyboard->buffer_head = (keyboard->buffer_head + 1) & 0x0F;
                printf("Key released: scan code 0x%02X\n", scancode);
            }
            break;
    }
}

/* Check if keyboard has a key ready */
bool xt_keyboard_has_key(struct xt_keyboard *keyboard) {
    return keyboard->buffer_head != keyboard->buffer_tail;
}

/* Read scan code from keyboard buffer */
uint8_t xt_keyboard_read_scancode(struct xt_keyboard *keyboard) {
    if (keyboard->buffer_head != keyboard->buffer_tail) {
        uint8_t scancode = keyboard->keyboard_buffer[keyboard->buffer_tail];
        keyboard->buffer_tail = (keyboard->buffer_tail + 1) & 0x0F;
        return scancode;
    }
    return 0x00; /* No key available */
}

/* Trigger keyboard interrupt */
void xt_keyboard_trigger_irq(struct xt_keyboard *keyboard, struct xt_pic *pic) {
    if (keyboard->keyboard_enabled && xt_keyboard_has_key(keyboard)) {
        xt_pic_trigger_irq(pic, 1); /* IRQ 1 - Keyboard */
    }
}

/* PIT (Programmable Interval Timer) implementation */

/* PIT modes */
#define XT_PIT_MODE0  0x00  /* Interrupt on terminal count */
#define XT_PIT_MODE1  0x01  /* Hardware retriggerable one-shot */
#define XT_PIT_MODE2  0x02  /* Rate generator */
#define XT_PIT_MODE3  0x03  /* Square wave generator */
#define XT_PIT_MODE4  0x04  /* Software triggered strobe */
#define XT_PIT_MODE5  0x05  /* Hardware triggered strobe */

/* PIT command register bits */
#define XT_PIT_CMD_SC   0x11  /* Select counter */
#define XT_PIT_CMD_RW   0x12  /* Read/Write mode */
#define XT_PIT_CMD_MODE 0x13  /* Operating mode */
#define XT_PIT_CMD_BCD  0x14  /* Binary/BCD count */

/* Initialize PIT */
void xt_pit_init(struct xt_pit *pit) {
    /* Clear all PIT registers */
    memset(pit, 0, sizeof(struct xt_pit));
    
    /* Initialize channels */
    for (int i = 0; i < 3; i++) {
        pit->channels[i].counter = 0xFFFF;
        pit->channels[i].latch = 0xFFFF;
        pit->channels[i].mode = XT_PIT_MODE0;
        pit->channels[i].bcd = false;
        pit->channels[i].read_back = false;
        pit->channels[i].status = 0x00;
    }
    
    /* Set control register */
    pit->control = 0x00;
    
    /* Set initial state */
    pit->initialized = true;
    pit->last_tick = 0;
    pit->tick_count = 0;
    pit->timer_running = false;
    
    printf("PIT initialized\n");
}

/* Write to PIT register */
void xt_pit_write(struct xt_pit *pit, uint16_t port, uint8_t value) {
    uint8_t channel = port - XT_PIT_BASE;
    
    if (channel > 2) {
        return; /* Invalid channel */
    }
    
    if (port == XT_PIT_BASE) {
        /* Control register write */
        pit->control = value;
        
        uint8_t sc = (value >> 6) & 0x03;    /* Select counter */
        uint8_t rw = (value >> 4) & 0x03;    /* Read/Write mode */
        uint8_t mode = (value >> 1) & 0x07;  /* Operating mode */
        uint8_t bcd = (value & 0x01);        /* Binary/BCD */
        
        printf("PIT control write: SC=%d, RW=%d, Mode=%d, BCD=%d\n", sc, rw, mode, bcd);
        
        if (sc < 3) {
            pit->channels[sc].command = value;
            pit->channels[sc].mode = mode;
            pit->channels[sc].bcd = bcd;
        }
    } else {
        /* Counter register write */
        uint8_t counter = channel;
        
        switch (pit->channels[counter].command & 0x30) {
            case 0x00: /* Latch counter only */
                pit->channels[counter].latch = value;
                break;
                
            case 0x10: /* Write low byte only */
                pit->channels[counter].counter = (pit->channels[counter].counter & 0xFF00) | value;
                break;
                
            case 0x20: /* Write high byte only */
                pit->channels[counter].counter = (pit->channels[counter].counter & 0x00FF) | (value << 8);
                break;
                
            case 0x30: /* Write low then high byte */
                pit->channels[counter].counter = (pit->channels[counter].counter & 0x00FF) | (value << 8);
                break;
        }
        
        printf("PIT channel %d counter write: 0x%04X\n", counter, pit->channels[counter].counter);
        
        /* Start timer if channel 0 is programmed */
        if (counter == 0 && pit->channels[0].counter > 0) {
            pit->timer_running = true;
            pit->last_tick = 0;
            printf("PIT timer started with counter: %d\n", pit->channels[0].counter);
        }
    }
}

/* Read from PIT register */
uint8_t xt_pit_read(struct xt_pit *pit, uint16_t port) {
    uint8_t channel = port - XT_PIT_BASE;
    
    if (channel > 2) {
        return 0xFF; /* Invalid channel */
    }
    
    if (port == XT_PIT_BASE) {
        /* Control register read - return status */
        return pit->control;
    } else {
        /* Counter register read */
        uint8_t counter = channel;
        uint8_t value = 0;
        
        switch (pit->channels[counter].command & 0x30) {
            case 0x00: /* Latch counter only */
                value = pit->channels[counter].latch & 0xFF;
                break;
                
            case 0x10: /* Read low byte only */
                value = pit->channels[counter].counter & 0xFF;
                break;
                
            case 0x20: /* Read high byte only */
                value = (pit->channels[counter].counter >> 8) & 0xFF;
                break;
                
            case 0x30: /* Read low byte then high byte */
                value = pit->channels[counter].counter & 0xFF;
                break;
        }
        
        printf("PIT channel %d counter read: 0x%02X\n", counter, value);
        return value;
    }
}

/* Update PIT state and check for timer interrupts */
void xt_pit_update(struct xt_pit *pit, uint32_t cycles) {
    if (!pit->timer_running || pit->channels[0].counter == 0) {
        return;
    }
    
    /* Channel 0 is used for system timer interrupts */
    uint16_t channel0_counter = pit->channels[0].counter;
    
    /* Calculate timer frequency: 1.193182 MHz XT clock */
    /* Timer interrupt occurs every 65536 cycles at maximum count */
    uint32_t timer_threshold = channel0_counter;
    
    pit->tick_count += cycles;
    
    /* Check if timer interrupt should be triggered */
    if (pit->tick_count >= timer_threshold) {
        pit->tick_count = 0;
        printf("PIT timer interrupt triggered (counter: %d)\n", channel0_counter);
    }
}

/* Trigger IRQ 0 (timer interrupt) */
void xt_pit_trigger_irq0(struct xt_pit *pit, struct xt_pic *pic) {
    if (pit->timer_running && pit->channels[0].counter > 0) {
        xt_pic_trigger_irq(pic, 0); /* IRQ 0 - System Timer */
        printf("PIT triggered IRQ 0\n");
    }
}