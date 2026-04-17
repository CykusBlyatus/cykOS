#ifndef CYKOS_VIRTIO_MMIO_H
#define CYKOS_VIRTIO_MMIO_H

#include <stdint.h>

// VirtIO registers relative to base address

#define VIRTIO_MMIO_MAGIC		     0x000 // 0x74726976
#define VIRTIO_MMIO_VERSION		     0x004 // version; should be 2
#define VIRTIO_MMIO_DEVICE_TYPE      0x008 // device type; 1 is net, 2 is disk
#define VIRTIO_MMIO_VENDOR_ID		 0x00c // 0x554d4551
#define VIRTIO_MMIO_DEVICE_FEATURES	 0x010 // available device features
#define VIRTIO_MMIO_DEV_FEAT_SEL     0x014 // bit b at VIRTIO_MMIO_DEVICE_FEATURES refers to bit VIRTIO_MMIO_DEV_FEAT_SEL*32+b
#define VIRTIO_MMIO_DRIVER_FEATURES	 0x020 // chosen driver features
#define VIRTIO_MMIO_DRV_FEAT_SEL     0x024 // bit b at VIRTIO_MMIO_DRIVER_FEATURES refers to bit VIRTIO_MMIO_DRV_FEAT_SEL*32+b
#define VIRTIO_MMIO_QUEUE_SEL		 0x030 // select queue, write-only
#define VIRTIO_MMIO_QUEUE_SIZE_MAX	 0x034 // max size of current queue, read-only
#define VIRTIO_MMIO_QUEUE_SIZE		 0x038 // size of current queue, write-only
#define VIRTIO_MMIO_QUEUE_READY		 0x044 // ready bit
#define VIRTIO_MMIO_QUEUE_NOTIFY	 0x050 // write-only
#define VIRTIO_MMIO_INTERRUPT_STATUS 0x060 // read-only
#define VIRTIO_MMIO_INTERRUPT_ACK	 0x064 // write-only
#define VIRTIO_MMIO_STATUS		     0x070 // read/write
#define VIRTIO_MMIO_QUEUE_DESC_LOW	 0x080 // physical address for descriptor table, write-only
#define VIRTIO_MMIO_QUEUE_DESC_HIGH	 0x084
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW	 0x090 // physical address for available ring, write-only
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH 0x094
#define VIRTIO_MMIO_QUEUE_USED_LOW	 0x0a0 // physical address for used ring, write-only
#define VIRTIO_MMIO_QUEUE_USED_HIGH  0x0a4
#define VIRTIO_MMIO_CONFIG           0x100

// Value that should be at base+0x0 (VIRTIO_MMIO_MAGIC)
#define VIRTIO_MAGIC_VALUE 0x74726976

// Expected VirtIO version
#define VIRTIO_VERSION 2

// VirtIO device type at base+0x8 (VIRTIO_MMIO_DEVICE_TYPE)
#define VIRTIO_DEVTYPE_NET          1
#define VIRTIO_DEVTYPE_BLOCK        2
#define VIRTIO_DEVTYPE_CONSOLE      3
#define VIRTIO_DEVTYPE_ENTROPY      4
#define VIRTIO_DEVTYPE_MEMBALLOON   5
#define VIRTIO_DEVTYPE_SCSI         8
#define VIRTIO_DEVTYPE_GPU          16
#define VIRTIO_DEVTYPE_INPUT        18
#define VIRTIO_DEVTYPE_CRYPTO       20
#define VIRTIO_DEVTYPE_SOCKET       19
#define VIRTIO_DEVTYPE_FILESYS      26
#define VIRTIO_DEVTYPE_RPMB         28
#define VIRTIO_DEVTYPE_IOMMU        23
#define VIRTIO_DEVTYPE_SOUND        25
#define VIRTIO_DEVTYPE_MEMORY       24
#define VIRTIO_DEVTYPE_I2C          34
#define VIRTIO_DEVTYPE_SCMI         32
#define VIRTIO_DEVTYPE_GPIO         41
#define VIRTIO_DEVTYPE_PMEM         27

// VirtIO device status at base+0x70 (VIRTIO_MMIO_STATUS)
#define VIRTIO_STATUS_ACK           1
#define VIRTIO_STATUS_DRIVER		2
#define VIRTIO_STATUS_DRIVER_OK     4
#define VIRTIO_STATUS_FEATURES_OK	8
#define VIRTIO_STATUS_NEEDS_RESET   64
#define VIRTIO_STATUS_FAILED        128

// Device-independent feature bits
#define VIRTIO_F_INDIRECT_DESC      (1L << 28)
#define VIRTIO_F_EVENT_IDX          (1L << 29)
#define VIRTIO_F_VERSION_1          (1L << 32)
#define VIRTIO_F_ACCESS_PLATFORM    (1L << 33)
#define VIRTIO_F_RING_PACKED        (1L << 34)
#define VIRTIO_F_IN_ORDER           (1L << 35)
#define VIRTIO_F_ORDER_PLATFORM     (1L << 36)
#define VIRTIO_F_SR_IOV             (1L << 37)
#define VIRTIO_F_NOTIFICATION_DATA  (1L << 38)
#define VIRTIO_F_NOTIF_CONFIG_DATA  (1L << 39)
#define VIRTIO_F_RING_RESET         (1L << 40)
#define VIRTIO_F_ADMIN_VQ           (1L << 41)

#endif
