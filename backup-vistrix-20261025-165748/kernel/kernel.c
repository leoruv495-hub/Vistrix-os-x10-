#include <stdint.h>
#include "vga.h"
#include "serial.h"
#include "keyboard.h"

void kernel_main(void)
{
    serial_init();
    serial_write("\nVistrix OS X10: kernel avviato.\n");
    serial_write("Modalita: kernel freestanding 32-bit.\n");

    vga_clear(0x1F);
    vga_write_at("VISTRIX OS X10", 0x1F, 2, 2);
    vga_write_at("==============================", 0x1F, 2, 3);
    vga_write_at("Sistema avviato correttamente.", 0x1F, 2, 6);
    vga_write_at("Kernel: Vistrix Core", 0x1F, 2, 8);
    vga_write_at("Driver VGA: inizializzato", 0x1F, 2, 10);
    vga_write_at("Driver seriale: inizializzato", 0x1F, 2, 11);
    vga_write_at("Driver tastiera: polling base", 0x1F, 2, 12);
    vga_write_at("Prossimo passo: memoria e GUI.", 0x1F, 2, 15);

    serial_write("Driver VGA: pronto.\n");
    serial_write("Driver seriale: pronto.\n");
    serial_write("Driver tastiera: polling base pronto.\n");
    serial_write("VISTRIX_BOOT_OK\n");

    for (;;) {
        /* Lettura base dei codici tastiera; non gestisce ancora gli IRQ. */
        (void)keyboard_poll();
        __asm__ volatile ("hlt");
    }
}
