.section .init
.global _start

_start:
    la sp,stack_top # initialize stack pointer
    call main

.section .bss
.align 8
stack_bottom:
.space 4096
stack_top:
