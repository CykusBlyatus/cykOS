#define DEBUG
#include <auxiliary/debug.h>

#include "virtio.h"
#include "mmio.h"
#include "virtq.h"
#include "blk.h"
#include <devices/syscon/syscon.h>
#include <memory/paging.h>
#include <string.h>
#include <stdio.h>

#define VIRTQ_SIZE 8
#define BUFSIZE 512

typedef struct disk {
    virtq_desc_t *desc;
    VIRTQ_AVAIL_T(VIRTQ_SIZE) *avail;
    VIRTQ_USED_T(VIRTQ_SIZE) *used;
    VIRTIO_BLK_REQ_T(BUFSIZE) reqs[VIRTQ_SIZE];
} disk_t;

disk_t disk;

void disk0_init() {
    if (DISK0(VIRTIO_MMIO_MAGIC) != VIRTIO_MAGIC_VALUE)
        panic("magic number at virtIO base address not %p (is %p)", (void*)VIRTIO_MAGIC_VALUE, (void*)(long)DISK0(VIRTIO_MMIO_MAGIC));
    if (DISK0(VIRTIO_MMIO_VERSION) != VIRTIO_VERSION)
        panic("VirtIO version not supported (supports %u, got %u)", VIRTIO_VERSION, DISK0(VIRTIO_MMIO_VERSION));
    if (DISK0(VIRTIO_MMIO_DEVICE_TYPE) != VIRTIO_DEVTYPE_BLOCK)
        panic("VirtIO device at %p not a block (disk) device (instead has device type %d)", &DISK0(0), DISK0(VIRTIO_MMIO_DEVICE_TYPE));

    DISK0(VIRTIO_MMIO_STATUS) = 0;
    while (DISK0(VIRTIO_MMIO_STATUS) != 0);

    DISK0(VIRTIO_MMIO_STATUS) |= VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER;

    DISK0(VIRTIO_MMIO_DEV_FEAT_SEL) = 0;
    u64 driver_features = DISK0(VIRTIO_MMIO_DEVICE_FEATURES);
    DISK0(VIRTIO_MMIO_DEV_FEAT_SEL) = 1;
    driver_features |= ((u64) DISK0(VIRTIO_MMIO_DEVICE_FEATURES)) << 32;

    driver_features &= ~(0
        | VIRTIO_BLK_F_RO
        | VIRTIO_BLK_F_SCSI
        | VIRTIO_BLK_F_CONFIG_WCE
        | VIRTIO_BLK_F_MQ
    );

    driver_features |= 0
        | VIRTIO_F_NOTIFICATION_DATA
        | VIRTIO_F_NOTIF_CONFIG_DATA
        ;

    DISK0(VIRTIO_MMIO_DRV_FEAT_SEL) = 0;
    DISK0(VIRTIO_MMIO_DRIVER_FEATURES) = driver_features;
    DISK0(VIRTIO_MMIO_DRV_FEAT_SEL) = 1;
    DISK0(VIRTIO_MMIO_DRIVER_FEATURES) = driver_features >> 32;

    DISK0(VIRTIO_MMIO_STATUS) |= VIRTIO_STATUS_FEATURES_OK;
    if (!(DISK0(VIRTIO_MMIO_STATUS) & VIRTIO_STATUS_FEATURES_OK))
        panic("virtio0 driver features were not accepted");

    virtio_blk_config_t *diskcfg = (void*)&DISK0(VIRTIO_MMIO_CONFIG);
    u64 capacity = diskcfg->capacity;
    u64 b = 512 * (capacity % 2);
    u64 kb = ((capacity >> 1) % 1024);
    u64 mb = ((capacity >> 11) % 1024);
    u64 gb = ((capacity >> 21) % 1024);
    DEBUG_INFO_NO_NEWLINE("Disk capacity is");
    if (gb)
        DEBUG_PRINTF_RAW(" %lluGB", (long long unsigned)gb);
    if (mb)
        DEBUG_PRINTF_RAW(" %lluMB", (long long unsigned)mb);
    if (kb)
        DEBUG_PRINTF_RAW(" %lluKB", (long long unsigned)kb);
    if (b)
        DEBUG_PRINTF_RAW(" %lluB", (long long unsigned)b);
    DEBUG_PRINTF_RAW("\n");

    disk = (disk_t) {
        .desc = pgallocdirect(PTE_RW),
        .avail = pgallocdirect(PTE_RW),
        .used = pgallocdirect(PTE_RW),
    };

    if (!disk.desc || !disk.avail || !disk.used)
        panic("Failed to allocate pages for disk virtqueues");

    memset(disk.desc, 0, PGSIZE);
    memset(disk.avail, 0, PGSIZE);
    memset(disk.used, 0, PGSIZE);

    DISK0(VIRTIO_MMIO_QUEUE_SEL) = 0;

    u32 maxqsize = DISK0(VIRTIO_MMIO_QUEUE_SIZE_MAX);
    if (maxqsize == 0) {
        panic("VirtIO disk has no queue 0");
    } else if (maxqsize < VIRTQ_SIZE) {
        panic("VirtIO disk max virtqueue size too small (%d < %d)", maxqsize, VIRTQ_SIZE);
    }

    DISK0(VIRTIO_MMIO_QUEUE_SIZE) = VIRTQ_SIZE;

    DISK0(VIRTIO_MMIO_QUEUE_DESC_HIGH) = VA2PA(disk.desc) >> 32;
    DISK0(VIRTIO_MMIO_QUEUE_DESC_LOW) = VA2PA(disk.desc);

    DISK0(VIRTIO_MMIO_QUEUE_AVAIL_HIGH) = VA2PA(disk.avail) >> 32;
    DISK0(VIRTIO_MMIO_QUEUE_AVAIL_LOW) = VA2PA(disk.avail);

    DISK0(VIRTIO_MMIO_QUEUE_USED_HIGH) = VA2PA(disk.used) >> 32;
    DISK0(VIRTIO_MMIO_QUEUE_USED_LOW) = VA2PA(disk.used);

    DISK0(VIRTIO_MMIO_QUEUE_READY) = 1;

    DISK0(VIRTIO_MMIO_STATUS) |= VIRTIO_STATUS_DRIVER_OK;

    disk.reqs[0].header = (virtio_blk_req_header_t) {
        .type = VIRTIO_BLK_T_IN,
        .sector = 0,
    };
    disk.reqs[0].status = 0xff;

    disk.desc[0].addr = (u64)&disk.reqs[0].header;
    disk.desc[0].len = sizeof(disk.reqs[0].header);
    disk.desc[0].flags = VIRTQ_DESC_F_NEXT;
    disk.desc[0].next = 1;

    disk.desc[1].addr = (u64)&disk.reqs[0].data;
    disk.desc[1].len = sizeof(disk.reqs[0].data);
    disk.desc[1].flags = VIRTQ_DESC_F_NEXT | VIRTQ_DESC_F_WRITE;
    disk.desc[1].next = 2;

    disk.desc[2].addr = (u64)&disk.reqs[0].status;
    disk.desc[2].len = 1;
    disk.desc[2].flags = VIRTQ_DESC_F_WRITE;
    disk.desc[2].next = 0;

    disk.avail->ring[0] = 0;
    disk.avail->idx++;

    // while (*(volatile u8*)&disk.reqs[0].status == 0xff);

    int i = 0;
    const int timeout = 100000000;
    for (i = 0; i < timeout && *(volatile u8*)&disk.reqs[0].status == 0xff; ++i);
    if (i >= timeout)
        panic("Disk took too long to reply");

    if (*(volatile u8*)&disk.reqs[0].status)
        panic("VirtIO replied to first read request with status 0x%x", disk.reqs[0].status);

    // const int bytes_per_line = 16;
    // DEBUG_INFO("First sector of disk0: ");
    // for (u8 *line = disk.reqs[0].data; line != disk.reqs[0].data + BUFSIZE; line += bytes_per_line) {
    //     for (u8 *byte = line; byte != line + bytes_per_line; ++byte) {
    //         DEBUG_PRINTF_RAW("0x%s%x ", *byte < 0x10 ? "0" : "", *byte);
    //     }
    //     DEBUG_PRINTF_RAW("\n");
    // }
    // DEBUG_PRINTF_RAW("\n");
}
