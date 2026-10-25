CC = gcc
AS = as
LD = ld

CFLAGS = -m32 -std=gnu11 -ffreestanding -fno-pie -fno-stack-protector \
         -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables \
         -Wall -Wextra -O2 -Iinclude
ASFLAGS = --32
LDFLAGS = -m elf_i386 -T linker.ld

OBJECTS = build/boot.o build/kernel.o build/mouse.o

.PHONY: all check iso run clean
all: iso

build:
	mkdir -p build

build/boot.o: boot/boot.s | build
	$(AS) $(ASFLAGS) $< -o $@

build/kernel.o: kernel/kernel.c include/mouse.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/mouse.o: drivers/mouse.c include/mouse.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.elf: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

check: build/kernel.elf
	grub-file --is-x86-multiboot build/kernel.elf
	@echo "OK: GRUB riconosce il kernel Multiboot."

iso: check
	mkdir -p iso/boot/grub
	cp build/kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o build/vistrix.iso iso
	@echo "ISO creata: build/vistrix.iso"

run: iso
	qemu-system-i386 -display none -cdrom build/vistrix.iso -m 128M -serial stdio -no-reboot -no-shutdown

clean:
	rm -rf build
