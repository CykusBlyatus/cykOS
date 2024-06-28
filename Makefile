# Compiler and Assembler
CC = riscv64-unknown-elf-gcc
AS = riscv64-unknown-elf-as

# C and Assembly flags
CFLAGS=-Wall -mcmodel=medany -nostdlib -nostartfiles -nodefaultlibs
ASFLAGS=

bootloader.bin: startup.o main.o linker.ld
	riscv64-unknown-elf-gcc -o bootloader.elf -T linker.ld startup.o main.o -nostdlib -nostartfiles -nodefaultlibs
	riscv64-unknown-elf-objcopy -O binary bootloader.elf bootloader.bin

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# I don't use this because it gets freaky, I recommend executing the command without Makefile
run:
	@qemu-system-riscv64 -nographic -machine virt -bios none -kernel bootloader.bin

clean:
	rm *.o bootloader.bin bootloader.elf >/dev/null 2>&1 || true
