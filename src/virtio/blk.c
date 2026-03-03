#define DEBUG
#include <auxiliary/debug.h>

#include "virtio.h"
#include "mmio.h"
#include "blk.h"
#include <syscon.h>
#include <paging.h>

void disk0_init() {
    pgmap(&kernel_pgdir, &DISK0(0), &DISK0(0), PTE_RW);

    if (DISK0(VIRTIO_MMIO_MAGIC) != VIRTIO_MAGIC_VALUE) {
        DEBUG_ERROR("magic number at virtIO base address not %p (is %p)", (void*)VIRTIO_MAGIC_VALUE, (void*)(long)DISK0(VIRTIO_MMIO_MAGIC));
        poweroff();
    }
    if (DISK0(VIRTIO_MMIO_VERSION) != VIRTIO_VERSION) {
        DEBUG_ERROR("VirtIO version not supported (supports %u, got %u)", VIRTIO_VERSION, DISK0(VIRTIO_MMIO_VERSION));
        poweroff();
    }
    if (DISK0(VIRTIO_MMIO_DEVICE_TYPE) != VIRTIO_DEVTYPE_BLOCK) {
        DEBUG_ERROR("VirtIO device at %p not a block (disk) device (instead has device type %d)", &DISK0(0), DISK0(VIRTIO_MMIO_DEVICE_TYPE));
        poweroff();
    }

    DISK0(VIRTIO_MMIO_STATUS) = 0;
    while (DISK0(VIRTIO_MMIO_STATUS) != 0);

    DISK0(VIRTIO_MMIO_STATUS) = VIRTIO_STATUS_ACK;
    DISK0(VIRTIO_MMIO_STATUS) = VIRTIO_STATUS_DRIVER;

    DISK0(VIRTIO_MMIO_DEV_FEAT_SEL) = 0;
    u64 device_features = DISK0(VIRTIO_MMIO_DEVICE_FEATURES);
    DISK0(VIRTIO_MMIO_DEV_FEAT_SEL) = 1;
    device_features |= ((u64) DISK0(VIRTIO_MMIO_DEVICE_FEATURES)) << 32;

    u32 driver_features = device_features  // disable device-independent features for now
        &~VIRTIO_BLK_F_RO
        &~VIRTIO_BLK_F_SCSI
        &~VIRTIO_BLK_F_CONFIG_WCE
        &~VIRTIO_BLK_F_MQ
        ;

    DISK0(VIRTIO_MMIO_DRV_FEAT_SEL) = 0;
    DISK0(VIRTIO_MMIO_DRIVER_FEATURES) = driver_features;

    DISK0(VIRTIO_MMIO_STATUS) |= VIRTIO_STATUS_FEATURES_OK;
    if (!(DISK0(VIRTIO_MMIO_STATUS) & VIRTIO_STATUS_FEATURES_OK)) {
        DEBUG_ERROR("virtio0 driver features were not accepted\n");
        poweroff();
    }

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
}
