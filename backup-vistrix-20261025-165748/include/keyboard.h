#ifndef VISTRIX_KEYBOARD_H
#define VISTRIX_KEYBOARD_H

#include <stdint.h>

/* Restituisce un codice di scansione, oppure 0 se non disponibile. */
uint8_t keyboard_poll(void);

#endif
