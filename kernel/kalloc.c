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

#define LOCKNAMEBUFSZ 6
char lockname[NCPU][LOCKNAMEBUFSZ];

struct run {
  struct run *next;
};

struct {
  struct spinlock locks[NCPU];
  struct run *freelists[NCPU];
  struct spinlock oplk;
} kmem;

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    snprintf(lockname[i], LOCKNAMEBUFSZ, "kmem%d", i);
    initlock(&kmem.locks[i], lockname[i]);
    kmem.freelists[i] = 0;
  }
  initlock(&kmem.oplk, "kmem");
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

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int hart = cpuid();
  acquire(&kmem.locks[hart]);
  r->next = kmem.freelists[hart];
  kmem.freelists[hart] = r;
  release(&kmem.locks[hart]);
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
  int hart = cpuid();
  acquire(&kmem.locks[hart]);
  r = kmem.freelists[hart];
  if(r)
    kmem.freelists[hart] = r->next;
  release(&kmem.locks[hart]);
  if(r)
    goto rfound;

  // try to steal mem from other harts
  acquire(&kmem.oplk);
  int idx = 0;
  while (idx < NCPU) {
    acquire(&kmem.locks[idx]);
    if (kmem.freelists[idx]) {
      break;
    }
    release(&kmem.locks[idx]);
    idx++;
  }
  if(idx == NCPU) {
    release(&kmem.oplk);
    goto rnotfound;
  }
  acquire(&kmem.locks[hart]);

  // idx -> hart 
  struct run *fast, *slow;
  fast = slow = kmem.freelists[idx];
  while (fast) {
    fast = fast->next;
    if (fast) {
      fast = fast->next;
      slow = slow->next;
    }
  }
  kmem.freelists[hart] = kmem.freelists[idx];
  kmem.freelists[idx] = slow->next;
  slow->next = 0;
  release(&kmem.locks[idx]);
  r = kmem.freelists[hart];
  kmem.freelists[hart] = r->next;
  release(&kmem.locks[hart]);
  release(&kmem.oplk);

rfound:
  memset((char*)r, 5, PGSIZE); // fill with junk
rnotfound:
  pop_off();
  return (void*)r;
}
