// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

unsigned ref_count[PHYSTOP/PGSIZE];
struct spinlock ref_lock;

int kinit_done = 0;

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void ref_add(uint64 pa,int amount){
  acquire(&ref_lock);
  ref_count[(uint64)pa / PGSIZE]+=amount;
  release(&ref_lock);
}

int get_ref(uint64 pa){
  acquire(&ref_lock);
  int ans = ref_count[(uint64)pa / PGSIZE];
  release(&ref_lock);
  return ans;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&ref_lock, "ref_lock");
  freerange(end, (void*)PHYSTOP);
  kinit_done = 1;
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

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  if(kinit_done){
    ref_add((uint64)pa,-1);
  }
  int rc = get_ref((uint64)pa);
  if(rc<0){
    panic("kfree");
  }
  if(!kinit_done || rc <= 0){
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    memset((char *)r, 5, PGSIZE); // fill with junk
    ref_count[(uint64)r / PGSIZE] = 1;
  }
  release(&kmem.lock);
  return (void*)r;
}
