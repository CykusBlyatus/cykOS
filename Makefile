# Compiler and Assembler
CC = riscv64-unknown-elf-gcc
AS = riscv64-unknown-elf-as

# C and Assembly flags
CFLAGS+=-I . -Wall -Wextra -Werror -mcmodel=medany -nostdlib -nostartfiles -nodefaultlibs
ASFLAGS=

OBJ_FILES=main.o startup.o uart.o syscon.o stdio.o ctype.o

bootloader.bin: $(OBJ_FILES) linker.ld
	riscv64-unknown-elf-gcc -o bootloader.elf -T linker.ld $(OBJ_FILES) -nostdlib -nostartfiles -nodefaultlibs
	riscv64-unknown-elf-objcopy -O binary bootloader.elf bootloader.bin

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

run: bootloader.bin
	@qemu-system-riscv64 -nographic -machine virt -bios none -kernel bootloader.bin

clean:
	rm *.o bootloader.bin bootloader.elf >/dev/null 2>&1 || true
