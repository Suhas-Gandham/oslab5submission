#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

#define MAX_SEMAPHORES 3

struct semaphore {
  struct spinlock lock;
  int value;
};

struct semaphore semaphores[MAX_SEMAPHORES];

void
semaphore_init(void)
{
  for(int i = 0; i < MAX_SEMAPHORES; i++){
    initlock(&semaphores[i].lock, "semaphore");
    semaphores[i].value = 0;
  }
}

int
sem_init(int id, int value)
{
  if(id < 0 || id >= MAX_SEMAPHORES || value < 0)
    return -1;

  acquire(&semaphores[id].lock);
  semaphores[id].value = value;
  release(&semaphores[id].lock);

  return 0;
}

int
sem_wait(int id)
{
  if(id < 0 || id >= MAX_SEMAPHORES)
    return -1;

  acquire(&semaphores[id].lock);

  while(semaphores[id].value == 0){
    sleep_prepare(&semaphores[id]);
    release(&semaphores[id].lock);
    sleep();
    acquire(&semaphores[id].lock);
  }

  semaphores[id].value--;

  release(&semaphores[id].lock);

  return 0;
}

int
sem_signal(int id)
{
  if(id < 0 || id >= MAX_SEMAPHORES)
    return -1;

  acquire(&semaphores[id].lock);

  semaphores[id].value++;

  wakeup(&semaphores[id]);

  release(&semaphores[id].lock);

  return 0;
}
