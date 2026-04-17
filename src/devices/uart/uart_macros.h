#ifndef CYKOS_UART_MACROS_H
#define CYKOS_UART_MACROS_H

#define UART0_BASE ((volatile uint8_t*)0x10000000)
#define UART0_REG(offset) (*(UART0_BASE + offset))

// UART registers
#define UART0_RBR UART0_REG(0) // Receiver Buffer Register
#define UART0_THR UART0_REG(0) // Transmit Holding Register
#define UART0_IER UART0_REG(1) // Interrupt Enable Register
#define UART0_IIR UART0_REG(2) // Interrupt Identification Register
#define UART0_FCR UART0_REG(2) // FIFO Control Register
#define UART0_LCR UART0_REG(3) // Line Control Register
#define UART0_MCR UART0_REG(4) // Modem Control Register
#define UART0_LSR UART0_REG(5) // Line Status Register
#define UART0_MSR UART0_REG(6) // Modem Status Register
#define UART0_SR  UART0_REG(7) // Scratchpad Register



// LCR (Line Control Register) Bits
#define UART_LCR_BPC(n)     ((n) - 5)      // Bits Per Char (5,6,7 or 8)
#define UART_LCR_2SB        (1L << 1)      // Two stop bits
#define UART_LCR_PAR        (0b111L << 3)  // Parity mask
#define UART_LCR_PAR_NONE   (0b000L << 3)  // No parity
#define UART_LCR_PAR_ODD    (0b001L << 3)  // Odd parity
#define UART_LCR_PAR_EVEN   (0b011L << 3)  // Even parity
#define UART_LCR_PAR_1      (0b101L << 3)  // Always one
#define UART_LCR_PAR_0      (0b111L << 3)  // Always 0
#define UART_LCR_BRK_CTRL   (1L << 6)      // Break control
#define UART_LCR_BAUD       (1L << 7)      // Special mode to set baud rate
//#define UART_LCR_RBR_THR  0              // RBR or THR



// LSR (Line Status Register) Bits
#define UART_LSR_RECV       (1L << 0)  // Receiver data
#define UART_LSR_OVERRUN    (1L << 1)  // Overrun error
#define UART_LSR_PARITY     (1L << 2)  // Parity error
#define UART_LSR_FRAMING    (1L << 3)  // Framing error
#define UART_LSR_BREAK_INT  (1L << 4)  // Break interrupt
#define UART_LSR_THRE       (1L << 5)  // Transmitter Holding Register Empty
#define UART_LSR_TER        (1L << 6)  // Transmitter Empty Register
#define UART_LSR_FIFO_ERR   (1L << 7)  // FIFO error



// IER (Interrupt Enable Register) Bits
#define UART_IER_RECV   (1L << 0)  // Received Data Available interrupt
#define UART_IER_THRE   (1L << 1)  // Transmitter Holding Register Empty interrupt
#define UART_IER_RLS    (1L << 2)  // Receiver Line Status interrupt
#define UART_IER_MODEM  (1L << 3)  // MODEM Status interrupt



// IIR (Interrupt Identification Register) Bits
#define UART_IIR_NO_INT         (1L << 0)   // No interrupts
#define UART_IIR_THRE           (1L << 1)   // Transmitter Holding Register Empty
#define UART_IIR_RDA            (1L << 2)   // Received Data available
#define UART_IIR_RLS            (3L << 1)   // Receiver Line Status interrupt
#define UART_IIR_CHR_TIMEOUT    (3L << 2)   // Character Timemout
#define UART_IIR_FIFO64         (1L << 5)   // 64-byte FIFO enabled
#define UART_IIR_FCR_B0         (3L << 6)   // Bit 0 of FCR set
#define UART_IIR_MODEM          0           // MODEM Status



// FCR (FIFO Control Register) Bits
#define UART_FCR_ENABLE     (1L << 0) // Enable both FIFOs
#define UART_FCR_CLR_RCVR   (1L << 1) // Clear all bytes in RCVR FIFO
#define UART_FCR_CLR_XMIT   (1L << 2) // Clear all bytes in the XMIT FIFO
#define UART_FCR_64         (1L << 5) // Enable 64 byte FIFO
#define UART_FCR_RCVR_1B    0         // RCVR FIFO Trigger of one byte
#define UART_FCR_RCVR_4B    (1L << 6) // RCVR FIFO Trigger of four bytes
#define UART_FCR_RCVR_8B    (1L << 7) // RCVR FIFO Trigger of eight bytes
#define UART_FCR_RCVR_14B   (3L << 6) // RCVR FIFO Trigger of fourteen bytes
#define UART_FCR_CLR        (UART_FCR_CLR_RCVR | UART_FCR_CLR_XMIT)

#endif // CYKOS_UART_MACROS_H
