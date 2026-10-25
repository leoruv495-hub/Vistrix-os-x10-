#ifndef VISTRIX_MOUSE_H
#define VISTRIX_MOUSE_H

#include <stdint.h>

void mouse_init(void);
int mouse_poll(int *dx, int *dy, uint8_t *buttons);

#endif
