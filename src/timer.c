#include "timer.h"
#include "csr.h"

#define MTIMER_ADDR 0x2000000
#define MTIMECMP (*(uint64_t*)(MTIMER_ADDR + 0x4000))
#define MTIME    (*(uint64_t*)(MTIMER_ADDR + 0xBFF8))

void set_mtimecmp(uint64_t time) {
    MTIMECMP = time;
}

uint64_t mtime_() {
    return MTIME;
}

void mtimer_init() {

    // csrs("menvcfg", BIT(XLEN-1)); // unknown CSR 'menvcfg'
    // csrs("mcounteren", BIT(1));

    MTIMECMP = MTIME + 1000000;

    // csrs("mie", CSR_MIE_MTIE | CSR_MIE_STIE);
}