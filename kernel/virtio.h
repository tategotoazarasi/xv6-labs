#ifndef _VIRTIO_H_
#define _VIRTIO_H_
/// \file virtio.h
/// virtio device definitions.
/// for both the mmio interface, and virtio descriptors.
/// only tested with qemu.
/// this is the "legacy" virtio interface.
///
/// the virtio spec:
/// https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf

// virtio mmio control registers, mapped starting at 0x10001000.
// from qemu virtio_mmio.h
/// \brief 0x74726976
#define VIRTIO_MMIO_MAGIC_VALUE 0x000
/// \brief version; 1 is legacy
#define VIRTIO_MMIO_VERSION 0x004
/// \brief device type; 1 is net, 2 is disk
#define VIRTIO_MMIO_DEVICE_ID 0x008
/// \brief 0x554d4551
#define VIRTIO_MMIO_VENDOR_ID 0x00c
#define VIRTIO_MMIO_DEVICE_FEATURES 0x010
#define VIRTIO_MMIO_DRIVER_FEATURES 0x020
/// \brief page size for PFN, write-only
#define VIRTIO_MMIO_GUEST_PAGE_SIZE 0x028
/// \brief select queue, write-only
#define VIRTIO_MMIO_QUEUE_SEL 0x030
/// \brief max size of current queue, read-only
#define VIRTIO_MMIO_QUEUE_NUM_MAX 0x034
/// \brief size of current queue, write-only
#define VIRTIO_MMIO_QUEUE_NUM 0x038
/// \brief used ring alignment, write-only
#define VIRTIO_MMIO_QUEUE_ALIGN 0x03c
/// \brief physical page number for queue, read/write
#define VIRTIO_MMIO_QUEUE_PFN                                                  \
  0x040 // physical page number for queue, read/write
/// \brief ready bit
#define VIRTIO_MMIO_QUEUE_READY 0x044
/// \brief write-only
#define VIRTIO_MMIO_QUEUE_NOTIFY 0x050
/// \brief read-only
#define VIRTIO_MMIO_INTERRUPT_STATUS 0x060
/// \brief write-only
#define VIRTIO_MMIO_INTERRUPT_ACK 0x064
/// \brief read/write
#define VIRTIO_MMIO_STATUS 0x070

// status register bits, from qemu virtio_config.h
#define VIRTIO_CONFIG_S_ACKNOWLEDGE 1
#define VIRTIO_CONFIG_S_DRIVER 2
#define VIRTIO_CONFIG_S_DRIVER_OK 4
#define VIRTIO_CONFIG_S_FEATURES_OK 8

// device feature bits
/// \brief device feature bits Disk is read-only
#define VIRTIO_BLK_F_RO 5
/// \brief device feature bits Supports scsi command passthru
#define VIRTIO_BLK_F_SCSI 7
/// \brief device feature bits Writeback mode available in config
#define VIRTIO_BLK_F_CONFIG_WCE 11
/// \brief device feature bits support more than one vq
#define VIRTIO_BLK_F_MQ 12
#define VIRTIO_F_ANY_LAYOUT 27
#define VIRTIO_RING_F_INDIRECT_DESC 28
#define VIRTIO_RING_F_EVENT_IDX 29

/// \brief this many virtio descriptors.
/// must be a power of two.
#define NUM 8

/// \brief a single descriptor, from the spec.
struct virtq_desc {
  uint64 addr;
  uint32 len;
  uint16 flags;
  uint16 next;
};
/// \brief chained with another descriptor
#define VRING_DESC_F_NEXT 1
/// \brief device writes (vs read)
#define VRING_DESC_F_WRITE 2

/// \brief the (entire) avail ring, from the spec.
struct virtq_avail {
  uint16 flags;     ///< always zero
  uint16 idx;       ///< driver will write ring[idx] next
  uint16 ring[NUM]; ///< descriptor numbers of chain heads
  uint16 unused;
};

/// \brief one entry in the "used" ring, with which the
/// device tells the driver about completed requests.
struct virtq_used_elem {
  uint32 id; ///< index of start of completed descriptor chain
  uint32 len;
};

struct virtq_used {
  uint16 flags; ///< always zero
  uint16 idx;   ///< device increments when it adds a ring[] entry
  struct virtq_used_elem ring[NUM];
};

// these are specific to virtio block devices, e.g. disks,
// described in Section 5.2 of the spec.

/// \brief read the disk
#define VIRTIO_BLK_T_IN 0
/// \brief write the disk
#define VIRTIO_BLK_T_OUT 1

/// \brief the format of the first descriptor in a disk request.
/// to be followed by two more descriptors containing
/// the block, and a one-byte status.
struct virtio_blk_req {
  uint32 type; ///< VIRTIO_BLK_T_IN or ..._OUT
  uint32 reserved;
  uint64 sector;
};
#endif