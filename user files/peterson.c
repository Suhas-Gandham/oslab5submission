#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define ITERATIONS 10

struct shared_data {
  volatile int flag[2];
  volatile int turn;
  volatile int shared_counter;
};

static inline void
memory_barrier(void)
{
  __sync_synchronize();
}

void
enter_cs(struct shared_data *shared, int id)
{
  int other = 1 - id;

  shared->flag[id] = 1;
  memory_barrier();

  shared->turn = other;
  memory_barrier();

  while(shared->flag[other] == 1 && shared->turn == other)
    ;
}

void
leave_cs(struct shared_data *shared, int id)
{
  memory_barrier();
  shared->flag[id] = 0;
  memory_barrier();
}

void
delay(void)
{
  for(volatile int i = 0; i < 500000; i++)
    ;
}

int
main(void)
{
  struct shared_data *shared;
  int pid;
  int id;

  pid = fork();

  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  shared = (struct shared_data *)shm_get();

  if(shared == 0){
    printf("shm_get failed\n");
    exit(1);
  }

  if(pid == 0)
    id = 1;
  else
    id = 0;

  if(id == 0){
    shared->flag[0] = 0;
    shared->flag[1] = 0;
    shared->turn = 0;
    shared->shared_counter = 0;

    memory_barrier();
  }

  if(id == 0){
    for(volatile int i = 0; i < 100000; i++)
      ;
  }

  for(int i = 0; i < ITERATIONS; i++){
    enter_cs(shared, id);

    shared->shared_counter++;

    printf("Process %d ENTER CS, counter = %d\n",
           id, shared->shared_counter);

    for(volatile int j = 0; j < 100000; j++)
      ;

    printf("Process %d EXIT CS\n", id);

    leave_cs(shared, id);

    delay();
  }

  if(id == 0){
    wait(0);

    printf("Final counter = %d\n",
           shared->shared_counter);
  }

  exit(0);
}
