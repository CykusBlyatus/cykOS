#ifndef CYKOS_VIRTIO_BLK_H
#define CYKOS_VIRTIO_BLK_H

#include <stdint.h>

#define DISK0_IRQ 1

void virtio_blk_init();
// int virtio_blk_init(void *dev_addr);
int virtio_blk_rw(u64 buf_pa, u32 blockno, u8 write);
void virtio_blk_intr();

#endif
