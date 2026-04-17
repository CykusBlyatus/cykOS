#ifndef CYKOS_VIRTQ_H
#define CYKOS_VIRTQ_H

#include <stdint.h>

typedef struct virtq_desc {
    /* Address (guest-physical). */
    u64 addr;
    /* length. */
    u32 len;
    /* This marks a buffer as continuing via the next field. */
    #define VIRTQ_DESC_F_NEXT 1
    /* This marks a buffer as device write-only (otherwise device read-only). */
    #define VIRTQ_DESC_F_WRITE 2
    /* This means the buffer contains a list of buffer descriptors. */
    #define VIRTQ_DESC_F_INDIRECT 4
    /* The flags as indicated above. */
    u16 flags;
    /* Next field if flags & NEXT */
    u16 next;
} virtq_desc_t;

#define INDIRECT_DESC_TABLE_T(len) struct {\
    /* The actual descriptors (16 bytes each) */\
    struct virtq_desc desc[len / 16];\
}

#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
#define VIRTQ_AVAIL_T(virtq_size) struct {\
    u16 flags;\
    u16 idx;\
    u16 ring[virtq_size];\
    u16 used_event; /* Only if VIRTIO_F_EVENT_IDX */\
}

#define VIRTQ_USED_F_NO_NOTIFY 1
#define VIRTQ_USED_T(virtq_size) struct {\
    u16 flags;\
    u16 idx;\
    struct virtq_used_elem ring[virtq_size];\
    u16 avail_event; /* Only if VIRTIO_F_EVENT_IDX */\
}

/* u32 is used here for ids for padding reasons. */\
struct virtq_used_elem {\
    /* Index of start of used descriptor chain. */\
    u32 id;\
    /*\
    * The number of bytes written into the device writable portion of\
    * the buffer described by the descriptor chain.\
    */\
    u32 len;\
};

#endif
