#include <stdio.h>
#include "uart.h"
#include "syscon.h"

int main() {
    uart_init();
    int x = 5;
    printf("it's cockin time (btw x = %d and &x = %p)\n", x, &x);

    poweroff();
}
