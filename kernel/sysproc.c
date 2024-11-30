#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
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
  if(n < 0)
    n = 0;
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
  backtrace();
  return 0;
}

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

uint64
sys_sigalarm(void)
{
  int nticks;
  argint(0, &nticks);
  // printf("nticks: %d\n", nticks);
  myproc()->nticks = nticks;
  uint64 va;
  argaddr(1, &va);
  myproc()->handler = va;
  return nticks;
}

uint64
sys_sigreturn(void)
{
  struct proc* p = myproc();
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_sp = p->prevtrapframe->kernel_sp;
  p->trapframe->kernel_trap = p->prevtrapframe->kernel_trap;
  p->trapframe->epc = p->prevtrapframe->epc;
  p->trapframe->kernel_hartid = p->prevtrapframe->kernel_hartid;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->kernel_satp = p->prevtrapframe->kernel_satp;
  p->trapframe->ra = p->prevtrapframe->ra;
  p->trapframe->sp = p->prevtrapframe->sp;
  p->trapframe->gp = p->prevtrapframe->gp;
  p->trapframe->tp = p->prevtrapframe->tp;
  p->trapframe->t0 = p->prevtrapframe->t0;
  p->trapframe->t1 = p->prevtrapframe->t1;
  p->trapframe->t2 = p->prevtrapframe->t2;
  p->trapframe->s0 = p->prevtrapframe->s0;
  p->trapframe->s1 = p->prevtrapframe->s1;
  p->trapframe->a0 = p->prevtrapframe->a0;
  p->trapframe->a1 = p->prevtrapframe->a1;
  p->trapframe->a2 = p->prevtrapframe->a2;
  p->trapframe->a3 = p->prevtrapframe->a3;
  p->trapframe->a4 = p->prevtrapframe->a4;
  p->trapframe->a5 = p->prevtrapframe->a5;
  p->trapframe->a6 = p->prevtrapframe->a6;
  p->trapframe->a7 = p->prevtrapframe->a7;
  p->trapframe->s2 = p->prevtrapframe->s2;
  p->trapframe->s3 = p->prevtrapframe->s3;
  p->trapframe->s4 = p->prevtrapframe->s4;
  p->trapframe->s5 = p->prevtrapframe->s5;
  p->trapframe->s6 = p->prevtrapframe->s6;
  p->trapframe->s7 = p->prevtrapframe->s7;
  p->trapframe->s8 = p->prevtrapframe->s8;
  p->trapframe->s9 = p->prevtrapframe->s9;
  p->trapframe->s10 = p->prevtrapframe->s10;
  p->trapframe->s11 = p->prevtrapframe->s11;
  p->trapframe->t3 = p->prevtrapframe->t3;
  p->trapframe->t4 = p->prevtrapframe->t4;
  p->trapframe->t5 = p->prevtrapframe->t5;
  p->trapframe->t6 = p->prevtrapframe->t6;

  p->tickspassed = 0;
  return p->trapframe->a0;
}
