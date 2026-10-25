# Vistrix OS X10

Sistema operativo sperimentale x86 a 32 bit.

## Compilazione
make

## Controllo Multiboot
make check

## Avvio in QEMU
make run

## Componenti attuali
- Entry point assembly per GRUB Multiboot 1
- Kernel C freestanding
- Driver testo VGA
- Driver seriale COM1
- Polling base della tastiera PS/2

Questa è una base didattica: non include ancora multitasking,
gestione completa degli interrupt, memoria virtuale, filesystem,
USB o interfaccia grafica moderna.
