.section .init
.global _start

_start:
    csrr tp,mhartid # store hart ID
    la sp,stack_top # initialize stack pointer

    # sp = stack_top + 4096 * hart_id
    li t0,4096
    mul t0,t0,tp
    add sp,sp,t0

    j start

.section .bss
.align 8
stack_bottom:
.space 16 * 4096
stack_top:
