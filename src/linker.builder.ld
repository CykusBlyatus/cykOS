ENTRY(_start)

SECTIONS {
    . = 0x80000000; /* qemu kernel address */
    PROVIDE(kernel_start = .);

    . = ALIGN(PGSIZE);
    .text : {
        PROVIDE(kernel_text_start = .);
        *(.init*)
        *(.text*)
    }
    . = ALIGN(PGSIZE);
    PROVIDE(kernel_text_end = .);

    . = ALIGN(PGSIZE);
    .rodata : {
        PROVIDE(kernel_rodata_start = .);
        *(.rodata*)
        *(.srodata*)
    }
    . = ALIGN(PGSIZE);
    PROVIDE(kernel_rodata_end = .);

    . = ALIGN(PGSIZE);
    .data : {
        PROVIDE(kernel_data_start = .);
        *(.data)
        *(.sdata)
        PROVIDE(kernel_data_end = .);
    }

    .bss : {
        PROVIDE(kernel_bss_start = .);
        *(.bss)
        *(.sbss)
    }
    . = ALIGN(PGSIZE);
    PROVIDE(kernel_bss_end = .);

    PROVIDE(kernel_end = .);

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.riscv.attributes)
        *(.rela.*)
    }
}
