#define DEBUG
#include <auxiliary/debug.h>

#include "blk.h"
#include "virtio.h"
#include "mmio.h"
#include "virtq.h"
#include "blk_internal.h"
#include <devices/syscon/syscon.h>
#include <memory/paging.h>
#include <string.h>
#include <stdio.h>

#define VIRTQ_SIZE 8
#define BUFSIZE 512

typedef VIRTQ_AVAIL_T(VIRTQ_SIZE) virtq_avail_t;
typedef VIRTQ_USED_T(VIRTQ_SIZE) virtq_used_t;

typedef struct disk {
    virtq_desc_t *desc;
    virtq_avail_t *avail;
    virtq_used_t *used;

    virtio_blk_req_t reqs[VIRTQ_SIZE];

    u8 desc_inuse[VIRTQ_SIZE]; // bookkeeping
} disk_t;

static disk_t disk;

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
    // avoid floating-point operations
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
}

static int desc_alloc() {
    for (int i = 0; i < VIRTQ_SIZE; ++i) {
        if (!disk.desc_inuse[i]) {
            disk.desc_inuse[i] = 1;
            return i;
        }
    }
    return -1;
}

static void desc_free(int i) {
    if (i >= VIRTQ_SIZE)
        panic("virtio blk: attempted to free descriptor index (%u) >= VIRTQ_SIZE (%u)", i, VIRTQ_SIZE);
    if (!disk.desc_inuse[i])
        panic("virtio blk: attempted to free descriptor not in use (index %u)", i);
    disk.desc_inuse[i] = 0;
}

static int desc_alloc3(int idx[3]) {
    for (int i = 0; i < 3; ++i) {
        idx[i] = desc_alloc();
        if (idx[i] == -1) {
            for (int j = 0; j < i; ++j)
                desc_free(j);
            return -1;
        }
    }
    return 0;
}

int virtio_blk_rw(u64 buf_pa, u32 blockno, u8 write) {
    int idx[3];
    while (desc_alloc3(idx) != 0);
    DEBUG_INFO("%d %d %d", idx[0], idx[1], idx[2]);
    DEBUG_INFO("%p", (void*)buf_pa);

    virtio_blk_req_t *req = &disk.reqs[idx[0]];
    req->header = (virtio_blk_req_header_t) {
        .type = write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .sector = blockno,
    };
    req->status = 0xff;

    // header
    disk.desc[idx[0]] = (virtq_desc_t) {
        .addr = (u64)&req->header,
        .len = sizeof(req->header),
        .flags = VIRTQ_DESC_F_NEXT,
        .next = idx[1],
    };

    // data buffer
    disk.desc[idx[1]] = (virtq_desc_t) {
        .addr = buf_pa,
        .len = BUFSIZE,
        .flags = VIRTQ_DESC_F_NEXT | (write ? 0 : VIRTQ_DESC_F_WRITE),
        .next = idx[2],
    };

    // status
    disk.desc[idx[2]] = (virtq_desc_t) {
        .addr = (u64)&req->status,
        .len = 1,
        .flags = VIRTQ_DESC_F_WRITE,
        // .next = 0,
    };

    disk.avail->ring[disk.avail->idx % VIRTQ_SIZE] = idx[0];
    disk.avail->idx++;

    __sync_synchronize(); // ensure everything is ready before notifying disk

    DISK0(VIRTIO_MMIO_QUEUE_NOTIFY) = 0;

    u64 i = 0;
    const u64 timeout = 100000000;
    for (i = 0; i < timeout && *(volatile u8*)&disk.reqs[idx[0]].status == 0xff; ++i);
    if (i >= timeout)
        panic("Disk took too long to reply");

    if (*(volatile u8*)&disk.reqs[0].status)
        panic("VirtIO replied to request with status 0x%x", disk.reqs[0].status);

    for (int i = 0; i < 2; ++i)
        desc_free(idx[i]);

    return 0;
}
