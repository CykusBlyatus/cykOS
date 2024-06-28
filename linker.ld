ENTRY(_start)

SECTIONS {
    . = 0x80000000;          /* Load address for RISC-V */
    .text : {
        *(.init)
        *(.text*)
    }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
