#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  struct proc *p = myproc();
  // vmprint(p->pagetable);
  // virtual address to the buffer
  uint64 va;
  argaddr(0, &va);
  // printf("va:\t%p\n", va);

  // the number of pages
  int npages;
  argint(1, &npages);
  int upper_limit = 64;
  npages = npages > upper_limit? upper_limit: npages;

  // printf("npages:\t%d\n", npages);
  // pointer to bitmask
  uint64 vamask;
  argaddr(2, &vamask);
  // printf("vamask:\t%p\n", vamask);

  uint64 pamask = 0;
  int len = 8;

  for (int i = 0; i < npages; i++) {
    uint64 addr = va + i * PGSIZE;
    pte_t * pte = walk(p->pagetable, addr, 0);
    uint64 accessed = ((*pte) & PTE_A) >> 6;
    *pte &= (~PTE_A);
    pamask = pamask | (accessed << i);
    // printf("va %p pte %p PTE_A %d\n", addr, *pte, accessed);
  }
  // vmprint(p->pagetable);

  if (copyout(p->pagetable, vamask, (char *)&pamask, len) == 0) {
    return 0;
  }
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
