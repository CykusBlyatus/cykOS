/**
 * @file syscon.h
 * @author https://wiki.osdev.org/RISC-V_Meaty_Skeleton_with_QEMU_virt_board#src/syscon/syscon.h
 */

#ifndef SYSCON_H
#define SYSCON_H

// "test" syscon-compatible device is at memory-mapped address 0x100000
// according to our device tree
#define SYSCON_ADDR 0x100000

void poweroff(void);
void reboot(void);

#endif
