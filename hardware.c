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