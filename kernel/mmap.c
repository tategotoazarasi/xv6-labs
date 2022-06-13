#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "fcntl.h"
#include "proc.h"
#include "syscall.h"

uint64 alloc(pagetable_t pagetable, int size) {
  uint64 start = PGROUNDUP(0);
LOOP:
  for (uint64 m = start; m < start + size && m < MAXVA; m++) {
    if (!available_for_mmap(pagetable, m)) {
      start = PGROUNDUP(m + 1);
      goto LOOP;
    }
  }
  if (start + size > MAXVA) {
    return -1;
  }
  return start;
}

/// \brief mmap() creates a new mapping in the virtual address space of the
/// calling process. The starting address for the new mapping is specified in
/// addr. The length argument specifies the length of the mapping (which must be
/// greater than 0).
/// \return On success, mmap() returns a pointer to the mapped
/// area. On error, the value MAP_FAILED (that is, (void *) -1) is returned, and
/// errno is set to indicate the error.
uint64 sys_mmap(void) {
  // void *mmap(void *addr, int length, int prot, int flags, int fd, int
  // offset);
  uint64 addr; ///< You can assume that addr will always be zero, meaning that
               ///< the kernel should decide the virtual address at which to map
               ///< the file. mmap returns that address, or 0xffffffffffffffff
               ///< if it fails.
  int length;  ///< length is the number of bytes to map; it might not be the
               ///< same as the file's length.
  int prot;    ///< prot indicates whether the memory should be mapped readable,
               ///< writeable, and/or executable; you can assume that prot is
               ///< PROT_READ or PROT_WRITE or both.
  int flags; ///< flags will be either MAP_SHARED, meaning that modifications to
             ///< the mapped memory should be written back to the file, or
             ///< MAP_PRIVATE, meaning that they should not. You don't have to
             ///< implement any other bits in flags.
  // int fd;    ///< fd is the open file descriptor of the file to map.
  int offset; ///< You can assume offset is zero (it's the starting point in the
              ///< file at which to map).
  struct file *f;
  if (argaddr(0, &addr) < 0 || argint(1, &length) < 0 || argint(2, &prot) < 0 ||
      argint(3, &flags) < 0 || argfd(4, 0, &f) < 0 || argint(5, &offset) < 0) {
    return -1;
  }
  if (((prot & PROT_WRITE) && (!f->writable) &&
       (!(flags & MAP_PRIVATE) || (flags & MAP_SHARED))) ||
      ((prot & PROT_READ) && (!f->readable))) {
    return -1;
  }
  struct proc *p = myproc();
  int perm = PTE_U;
  if (prot & PROT_WRITE) {
    perm |= PTE_W;
  }
  if (prot & PROT_READ) {
    perm |= PTE_R;
  }
  if (prot & PROT_EXEC) {
    perm |= PTE_X;
  }
  uint64 va = alloc(p->pagetable, length);
  if (va < 0) {
    printf("mmap: alloc\n");
    return -1;
  }
  // printf("va = %d\n", va);
  if (mappages(p->pagetable, va, length, 0, PTE_U) < 0) {
    printf("mmap: mappages\n");
    return -1;
  }
  for (int i = 0; i < 16; i++) {
    if (!p->vmas[i].valid) {
      p->vmas[i].valid = 1;
      p->vmas[i].file = filedup(f);
      p->vmas[i].va = va;
      p->vmas[i].length = length;
      p->vmas[i].flags = flags;
      p->vmas[i].perm = perm;
      return va;
    }
  }
  return -1;
}

void write_back(struct file *f, pagetable_t pagetable, uint64 addr, int length,
                uint64 base) {
  //printf("base: %d, addr: %d, length: %d\n", base, addr, length);

  begin_op();
  for (uint64 m = addr; m < addr + length; m++) {
    // printf("write_back: m = %p\n", m);
    if (is_dirty(pagetable, m)) {
      //printf("write_back: DIRTY\n");
      ilock(f->ip);
      int r = writei(f->ip, 1, m, m - base, 1);
      if (r <= 0) {
        printf("sys_munmap: writei error %d\n", r);
        iunlockput(f->ip);
        end_op();
        return -1;
      }
      iunlock(f->ip);
    }
  }
  iupdate(f->ip);
  end_op();
}

/// \brief The munmap() system call deletes the mappings for the specified
/// address range, and causes further references to addresses within the range
/// to generate invalid memory references. The region is also automatically
/// unmapped when the process is terminated. On the other hand, closing the file
/// descriptor does not unmap the region.
/// \return On success, munmap() returns 0. On failure, it returns -1, and errno
/// is set to indicate the error (probably to EINVAL).
uint64 sys_munmap(void) {
  // int munmap(void *addr, int length);
  uint64 addr;
  int length;
  struct proc *p = myproc();
  if (argaddr(0, &addr) < 0 || argint(1, &length) < 0) {
    return -1;
  }
  for (int i = 0; i < 16; i++) {
    struct vma *vma = &p->vmas[i];
    if (vma->valid && vma->va <= addr &&
        addr + length <= vma->va + vma->length) {
      if (vma->flags == MAP_SHARED) {
        // write back
        write_back(vma->file, p->pagetable, addr, length, vma->va);
      }
      if (vma->va == addr && vma->length == length) {
        fileclose(vma->file);
        vma->valid = 0;
      } else if (vma->va == addr) {
        vma->va += length;
        vma->length -= length;
      } else if (addr + length == vma->va + vma->length) {
        vma->length -= length;
      } else {
        struct vma *newvma = 0;
        for (int j = 0; j < 16; j++) {
          if (!p->vmas[j].valid) {
            newvma = &p->vmas[j];
            break;
          }
        }
        if (!newvma) {
          printf("sys_munmap: no new available vma\n");
          return -1;
        }
        newvma->valid = 1;
        newvma->file = filedup(vma->file);
        newvma->va = addr + length;
        newvma->length = vma->va + vma->length - newvma->va;
        newvma->perm = vma->perm;
        newvma->flags = vma->flags;
        vma->length = addr - vma->va;
      }
    }
  }
  uvmunmap(p->pagetable, addr, PGROUNDUP(length) / PGSIZE, 0);
  return 0;
}
