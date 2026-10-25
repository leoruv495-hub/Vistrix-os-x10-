.section .multiboot,"a"
.align 4
.long 0x1BADB002
.long 0x00000007
.long -(0x1BADB002 + 0x00000007)
.long 0
.long 800
.long 600
.long 32

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
.extern kernel_main

_start:
    cli
    mov $stack_top, %esp
    xor %ebp, %ebp
    push %ebx
    push %eax
    call kernel_main

halt_loop:
    cli
    hlt
    jmp halt_loop

.size _start, . - _start
.section .note.GNU-stack,"",@progbits
