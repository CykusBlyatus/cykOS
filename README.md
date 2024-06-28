# cykOS

### Pre-requisites:
* A RISC-V compiler and assembler (preferably, riscv64-unknown-elf because it's the one in the Makefile)
* Something to run RISC-V on I guess (Makefile uses qemu-system-riscv64)

### Build:
Execute "make" in your terminal

### Run:
Execute __bootloader.bin__ (if you're using qemu-system-riscv64, just do ``qemu-system-riscv64 -nographic -machine virt -bios none -kernel bootloader.bin``)
