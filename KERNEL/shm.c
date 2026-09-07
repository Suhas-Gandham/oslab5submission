#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
struct {
  struct spinlock lock;
  uint64 pa;
} shm;

void
shm_init(void)
{
  initlock(&shm.lock, "shm");
  shm.pa = 0;
}

uint64
shm_get(void)
{
  struct proc *p = myproc();

  acquire(&shm.lock);

  if(shm.pa == 0){
    char *mem = kalloc();

    if(mem == 0){
      release(&shm.lock);
      return 0;
    }

    memset(mem, 0, PGSIZE);
    shm.pa = (uint64)mem;
  }

  if(mappages(p->pagetable, SHM_VA, PGSIZE,
              shm.pa, PTE_R | PTE_W | PTE_U) < 0){
    release(&shm.lock);
    return 0;
  }

  release(&shm.lock);

  return SHM_VA;
}

void
shm_unmap(pagetable_t pagetable)
{
  pte_t *pte;

  pte = walk(pagetable, SHM_VA, 0);

  if(pte != 0 && (*pte & PTE_V)){
    uvmunmap(pagetable, SHM_VA, 1, 0);
  }
}
