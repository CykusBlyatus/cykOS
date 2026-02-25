#include "timer.h"
#include "csr.h"

void mtimer_init() {
    mtimecmp = mtime + 10000000;
}

void stimer_init() {
  CSRS("menvcfg", 1L << (__riscv_xlen-1));
  CSRS("mcounteren", 1L << 1);

  CSRW("stimecmp", mtime + 10000000);
}
