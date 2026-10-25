CC = gcc
AS = as
LD = ld

CFLAGS = -m32 -std=gnu11 -ffreestanding -fno-pie -fno-stack-protector \
         -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables \
         -Wall -Wextra -O2 -Iinclude

ASFLAGS = --32
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL_OBJECTS = build/boot.o build/kernel.o \
                 build/vga.o build/serial.o build/keyboard.o

.PHONY: all check iso run clean

all: iso

build:
	mkdir -p build

build/boot.o: boot/boot.s | build
	$(AS) $(ASFLAGS) $< -o $@

build/kernel.o: kernel/kernel.c include/vga.h include/serial.h include/keyboard.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/vga.o: drivers/vga.c include/vga.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/serial.o: drivers/serial.c include/serial.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: drivers/keyboard.c include/keyboard.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.elf: $(KERNEL_OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJECTS)

check: build/kernel.elf
	grub-file --is-x86-multiboot build/kernel.elf
	@echo "OK: header Multiboot riconosciuto."

iso: check
	mkdir -p iso/boot/grub
	cp build/kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o build/vistrix.iso iso
	@echo "ISO pronta: build/vistrix.iso"

run: iso
	qemu-system-i386 -display none -cdrom build/vistrix.iso -m 128M -serial stdio -no-reboot -no-shutdown

clean:
	rm -rf build
