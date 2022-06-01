// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 31

char bcache_spinlock_names[NBUCKET][16];

typedef struct {
  uint8 len;
  struct buf bufs[NBUF];
  struct spinlock lock;
} buf_list;

struct {
  struct spinlock lock;
  buf_list list[NBUCKET];
} bcache;

void binit(void) {
  initlock(&bcache.lock,"bcache");
  /*// Create linked list of buffers
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }*/
  for (int i = 0; i < NBUCKET; i++) {
    snprintf(bcache_spinlock_names[i], 16, "bcache_%d_%d", i);
    initlock(&bcache.list[i].lock, bcache_spinlock_names[i]);
    bcache.list[i].len = 0;
  }
}

/// \brief Look through buffer cache for block on device dev.
/// If not found, allocate a buffer.
/// In either case, return locked buffer.
static struct buf *bget(uint dev, uint blockno) {
  struct buf *b;
  buf_list *l = &bcache.list[blockno % NBUF];
  acquire(&l->lock);
  for (int i = 0; i < l->len; i++) {
    if (l->bufs[i].dev == dev && l->bufs[i].blockno == blockno) {
      b = &l->bufs[i];
      b->refcnt++;
      goto RET;
    }
  }
  for (int i = 0; i < l->len; i++) {
    if (l->bufs[i].refcnt == 0) {
      b = &l->bufs[i];
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      goto RET;
    }
  }
  if(l->len==NBUF){
    panic("bget: no buffers");
  }
  b = &l->bufs[l->len++];
  b->dev = dev;
  b->blockno = blockno;
  b->valid = 0;
  b->refcnt = 1;
RET:
  release(&l->lock);
  acquiresleep(&b->lock);
  return b;
}

/// \brief Return a locked buf with the contents of the indicated block.
struct buf *bread(uint dev, uint blockno) {
  struct buf *b;

  b = bget(dev, blockno);
  if (!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

/// \brief Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b) {
  if (!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

/// \brief release a locked buffer.
/// Move to the head of the most-recently-used list.
void brelse(struct buf *b) {
  if (!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  buf_list* l = &bcache.list[b->blockno % NBUF];
  acquire(&l->lock);
  b->refcnt--;
  /*if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }*/

  release(&l->lock);
}

void bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}
