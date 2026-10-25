#include "mouse.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static int wait_input_clear(void)
{
    for (int i = 0; i < 100000; i++)
        if ((inb(0x64) & 2) == 0) return 1;
    return 0;
}

static int wait_output_full(void)
{
    for (int i = 0; i < 100000; i++)
        if (inb(0x64) & 1) return 1;
    return 0;
}

static void mouse_command(uint8_t value)
{
    if (!wait_input_clear()) return;
    outb(0x64, 0xD4);
    if (!wait_input_clear()) return;
    outb(0x60, value);
    if (wait_output_full()) (void)inb(0x60);
}

void mouse_init(void)
{
    if (wait_input_clear()) outb(0x64, 0xA8);

    if (wait_input_clear()) outb(0x64, 0x20);
    if (!wait_output_full()) return;

    uint8_t command = inb(0x60) | 2;
    if (wait_input_clear()) outb(0x64, 0x60);
    if (wait_input_clear()) outb(0x60, command);

    mouse_command(0xF6);
    mouse_command(0xF4);
}

int mouse_poll(int *dx, int *dy, uint8_t *buttons)
{
    static uint8_t packet[3];
    static int index = 0;

    if ((inb(0x64) & 1) == 0) return 0;

    uint8_t status = inb(0x64);
    uint8_t data = inb(0x60);

    /* Scarta i byte provenienti dalla tastiera. */
    if ((status & 0x20) == 0) return 0;

    if (index == 0 && (data & 0x08) == 0) return 0;
    packet[index++] = data;

    if (index < 3) return 0;
    index = 0;

    *dx = (int)(int8_t)packet[1];
    *dy = -(int)(int8_t)packet[2];
    *buttons = packet[0] & 7;
    return 1;
}
