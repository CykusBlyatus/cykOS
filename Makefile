# Compiler, Assembler and Linker
CC=riscv64-unknown-elf-gcc
AS=riscv64-unknown-elf-as
LD=riscv64-unknown-elf-ld

# C, Assembly and Linker flags
CFLAGS+=-I src/ -I src/libc/
CFLAGS+=-Wall -Wextra -Werror
CFLAGS+=-mcmodel=medany
CFLAGS+=-nostdlib -nostartfiles -nodefaultlibs -fno-builtin
CFLAGS+=-fno-pic -fno-pie

ASFLAGS+=

LDFLAGS+=-T $(LD_FILE)
LDFLAGS+=-no-pie -static
LDFLAGS+=--orphan-handling=error

# Source files
C_FILES=$(shell find src/ -name "*.c")
ASM_FILES=$(shell find src/ -name "*.S")
LD_FILE=src/linker.ld
LD_BUILDER=src/linker.builder.ld

# Object files
OBJS=$(C_FILES:src/%.c=obj/%.o) $(ASM_FILES:src/%.S=obj/%.o)

KERNEL=kernel.bin

DISK=disk.img
DISKID=hd0
DISKFMT=qcow2

all: $(KERNEL) $(DISK)
	@echo "CykOS ready!"

$(KERNEL): $(OBJS) $(LD_FILE)
	@echo "Linking to $@..."
	@$(LD) -o $(KERNEL) $(OBJS) $(LDFLAGS)

$(DISK):
	@echo "Creating disk ($@)..."
	@qemu-img create -f $(DISKFMT) $@ 5G > /dev/null

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

# Generate assembly files for analysis
local/asm/%.S: src/%.c
	@mkdir -p $(dir $@)
	@$(CC) -S $(CFLAGS) $< -o $@

$(LD_FILE): $(LD_BUILDER)
	@$(CC) -E -P -x c -D PGSIZE=0x1000 $(CFLAGS) $< > $@

run: all
	@qemu-system-riscv64 \
		-nographic \
		-machine virt \
		-bios none \
		-global virtio-mmio.force-legacy=false \
		-drive file=$(DISK),format=$(DISKFMT),if=none,id=$(DISKID) \
		-device virtio-blk-device,drive=$(DISKID),bus=virtio-mmio-bus.0 \
		-kernel $(KERNEL)

clean:
	@rm -rf obj $(KERNEL) $(LD_FILE) $(DISK)
