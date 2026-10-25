#include "vga.h"
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile uint16_t *const VGA_MEMORY =
    (volatile uint16_t *)0xB8000;

static int cursor_x = 0;
static int cursor_y = 0;

static uint16_t entry(char c, uint8_t color)
{
    return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_clear(uint8_t color)
{
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = entry(' ', color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_write_at(const char *text, uint8_t color, int x, int y)
{
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT)
        return;

    while (*text && y < VGA_HEIGHT) {
        if (*text == '\n') {
            x = 0;
            y++;
        } else if (x < VGA_WIDTH) {
            VGA_MEMORY[y * VGA_WIDTH + x] = entry(*text, color);
            x++;
        }
        text++;
    }
}

void vga_write(const char *text, uint8_t color)
{
    while (*text) {
        if (*text == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            VGA_MEMORY[cursor_y * VGA_WIDTH + cursor_x] =
                entry(*text, color);
            cursor_x++;
        }

        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }

        if (cursor_y >= VGA_HEIGHT) {
            cursor_x = 0;
            cursor_y = 0;
        }

        text++;
    }
}
