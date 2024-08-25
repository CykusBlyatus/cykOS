#ifndef CYKOS_ECALL_S_H
#define CYKOS_ECALL_S_H

enum {
    /**
     * @brief Tells M-mode that an interrupt has been handled
     * @param a0 Should contain the pending bit of the interrupt (e.g. STIP aka 0x20 for timer)
     */
    ECALL_S_INT_ACK,
};

#endif /* CYKOS_ECALL_S_H */
