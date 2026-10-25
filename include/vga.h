#ifndef VISTRIX_VGA_H
#define VISTRIX_VGA_H

#include <stdint.h>

void vga_clear(uint8_t color);
void vga_write_at(const char *text, uint8_t color, int x, int y);
void vga_write(const char *text, uint8_t color);

#endif
