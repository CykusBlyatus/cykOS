#include "timer.h"
#include "csr.h"

#define MTIMER_ADDR 0x2000000
#define MTIMECMP (*(uint64_t*)(MTIMER_ADDR + 0x4000))
#define MTIME    (*(uint64_t*)(MTIMER_ADDR + 0xBFF8))

uint64_t *mtimecmp_() {
    return &MTIMECMP;
}

uint64_t mtime_() {
    return MTIME;
}

void mtimer_init() {
    MTIMECMP = MTIME + 10000000;
}

void stimer_init() {
  csrs("menvcfg", BIT(__riscv_xlen-1));
  csrs("mcounteren", BIT(1));

  csrw("stimecmp", mtime + 10000000);
}
