/**
 * @file syscon.c
 * @author https://wiki.osdev.org/RISC-V_Meaty_Skeleton_with_QEMU_virt_board#src/syscon/syscon.c
*/

#include <stdio.h>
#include <stdint.h>
#include "syscon.h"

void poweroff(void) {
  puts("Poweroff requested");
  *(uint32_t *)SYSCON_ADDR = 0x5555;
}

void reboot(void) {
  puts("Reboot requested");
  *(uint32_t *)SYSCON_ADDR = 0x7777;
}
