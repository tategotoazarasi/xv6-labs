// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define INITLOCK(ii) initlock(&kmem.locks[ii], "kmem_"#ii);

void freerange(void *pa_start, void *pa_end);
char kmem_tag[NCPU][8];

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct spinlock locks[NCPU];
  struct run *freelist[NCPU];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  for(int i=0;i<NCPU;i++){
    snprintf(kmem_tag[i], 8, "kmem_%d", i);
    initlock(&kmem.locks[i], kmem_tag[i]);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  push_off();
  int cpu = cpuid();
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.locks[cpu]);
  r->next = kmem.freelist[cpu];
  kmem.freelist[cpu] = r;
  release(&kmem.locks[cpu]);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int cpu = cpuid();
  acquire(&kmem.locks[cpu]);
  r = kmem.freelist[cpu];
  if(r) {
    kmem.freelist[cpu] = r->next;
  } else {
    acquire(&kmem.lock);
    for(int i=0; i<NCPU; i++) {
      if(i!=cpu) {
        acquire(&kmem.locks[i]);
        if(kmem.freelist[i]) {
          r = kmem.freelist[i];
          //r->next = kmem.freelist[cpu];
          //kmem.freelist[cpu] = r;
          kmem.freelist[i] = kmem.freelist[i]->next;
          release(&kmem.locks[i]);
          break;
        }
        release(&kmem.locks[i]);
      }
    }
    release(&kmem.lock);
  }
  release(&kmem.locks[cpu]);
  pop_off();
  if(r) {
    memset((char *)r, 5, PGSIZE); // fill with junk
  }
  return (void*)r;
}