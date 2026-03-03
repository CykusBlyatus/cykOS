#ifndef CYKOS_VIRTIO_BLK_H
#define CYKOS_VIRTIO_BLK_H

#include <stdint.h>

#define QEMU_VIRTIO_DISK0 0x10001000
#define DISK0(offset) (*(uint32_t*)(QEMU_VIRTIO_DISK0 + offset))

// VirtIO block device feature bits

#define VIRTIO_BLK_F_BARRIER        (1 << 0)
#define VIRTIO_BLK_F_SIZE_MAX       (1 << 1)
#define VIRTIO_BLK_F_SEG_MAX        (1 << 2)
#define VIRTIO_BLK_F_GEOMETRY       (1 << 4)
#define VIRTIO_BLK_F_RO             (1 << 5)
#define VIRTIO_BLK_F_BLK_SIZE       (1 << 6)
#define VIRTIO_BLK_F_SCSI           (1 << 7)
#define VIRTIO_BLK_F_FLUSH          (1 << 9)
#define VIRTIO_BLK_F_TOPOLOGY       (1 << 10)
#define VIRTIO_BLK_F_CONFIG_WCE     (1 << 11)
#define VIRTIO_BLK_F_MQ             (1 << 12)
#define VIRTIO_BLK_F_DISCARD        (1 << 13)
#define VIRTIO_BLK_F_WRITE_ZEROES   (1 << 14)
#define VIRTIO_BLK_F_LIFETIME       (1 << 15)
#define VIRTIO_BLK_F_SECURE_ERASE   (1 << 16)
#define VIRTIO_BLK_F_ZONED          (1 << 17)

// VirtIO block device request types

#define VIRTIO_BLK_T_IN             0
#define VIRTIO_BLK_T_OUT            1
#define VIRTIO_BLK_T_FLUSH          4
#define VIRTIO_BLK_T_GET_ID         8
#define VIRTIO_BLK_T_GET_LIFETIME   10
#define VIRTIO_BLK_T_DISCARD        11
#define VIRTIO_BLK_T_WRITE_ZEROES   13
#define VIRTIO_BLK_T_SECURE_ERASE   14

typedef struct __attribute__((packed)) virtio_blk_config {
    u64 capacity;
    u32 size_max;
    u32 seg_max;
    struct virtio_blk_geometry {
        u16 cylinders;
        u8 heads;
        u8 sectors;
    } geometry;
    u32 blk_size;
    struct virtio_blk_topology {
        // # of logical blocks per physical block (log2)
        u8 physical_block_exp;
        // offset of first aligned logical block
        u8 alignment_offset;
        // suggested minimum I/O size in blocks
        u16 min_io_size;
        // optimal (suggested maximum) I/O size in blocks
        u32 opt_io_size;
    } topology;
    u8 writeback;
    u8 unused0;
    u16 num_queues;
    u32 max_discard_sectors;
    u32 max_discard_seg;
    u32 discard_sector_alignment;
    u32 max_write_zeroes_sectors;
    u32 max_write_zeroes_seg;
    u8 write_zeroes_may_unmap;
    u8 unused1[3];
    u32 max_secure_erase_sectors;
    u32 max_secure_erase_seg;
    u32 secure_erase_sector_alignment;
    struct virtio_blk_zoned_characteristics {
        u32 zone_sectors;
        u32 max_open_zones;
        u32 max_active_zones;
        u32 max_append_sectors;
        u32 write_granularity;
        u8 model;
        u8 unused2[3];
    } zoned;
} virtio_blk_config_t;

typedef struct __attribute__((packed)) virtio_blk_req {
    u32 type;
    u32 reserved;
    u64 sector;
    u8 data[];
} virtio_blk_req_t;

#endif
