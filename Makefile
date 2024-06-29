# Compiler and Assembler
CC = riscv64-unknown-elf-gcc
AS = riscv64-unknown-elf-as

# C, Assembly and Linker flags
CFLAGS=-I src/ -I src/libc/ -Wall -Wextra -Werror -mcmodel=medany
ASFLAGS=
LDFLAGS = -T src/linker.ld -nostdlib -nostartfiles -nodefaultlibs

# Source files
C_FILES=$(shell find src/ -name "*.c")
ASM_FILES=$(shell find src/ -name "*.S")

# Object files
OBJS=$(C_FILES:src/%.c=obj/%.o) $(ASM_FILES:src/%.S=obj/%.o)

TARGET=kernel.bin

$(TARGET): $(OBJS) src/linker.ld
	@mkdir -p obj
	@echo "Linking to $@..."
	@riscv64-unknown-elf-gcc -o $(TARGET) $(OBJS) $(LDFLAGS)

	@echo "CykOS ready!"

# Generic rule to compile C files
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $@..."
	@$(CC) -c $(CFLAGS) $< -o $@

# Generic rule for assembly files
obj/%.o: src/%.S
	@mkdir -p $(dir $@)
	@echo "Assembling $@..."
	@$(AS) $(ASFLAGS) $< -o $@

run: $(TARGET)
	@qemu-system-riscv64 -nographic -machine virt -bios none -kernel $(TARGET)

test:
	@echo "C_FILES: $(C_FILES)"
	@echo "ASM_FILES: $(ASM_FILES)"
	@echo "OBJS: $(OBJS)"

clean:
	rm -r obj $(TARGET) >/dev/null 2>&1 || true
