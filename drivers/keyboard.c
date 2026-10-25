#include "keyboard.h"

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

uint8_t keyboard_poll(void)
{
    /* Controller tastiera tradizionale PC/AT: polling senza IRQ. */
    if ((inb(0x64) & 1) == 0)
        return 0;

    return inb(0x60);
}
