#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PHILOSOPHERS 5
#define CYCLES 5

struct shared_table {
  int completed[NUM_PHILOSOPHERS];
};

void
delay(void)
{
  for(volatile int i = 0; i < 300000; i++)
    ;
}

void
philosopher(struct shared_table *table, int id)
{
  int left = id;
  int right = (id + 1) % NUM_PHILOSOPHERS;
  int first;
  int second;

  if(left < right){
    first = left;
    second = right;
  }else{
    first = right;
    second = left;
  }

  for(int cycle = 0; cycle < CYCLES; cycle++){

    printf("Philosopher %d: THINKING\n", id);
    delay();

    printf("Philosopher %d: HUNGRY\n", id);

    sem_wait(first);
    printf("Philosopher %d: picked fork %d\n", id, first);

    sem_wait(second);
    printf("Philosopher %d: picked fork %d\n", id, second);

    printf("Philosopher %d: EATING\n", id);
    delay();

    printf("Philosopher %d: finished eating\n", id);

    sem_signal(second);
    sem_signal(first);

    printf("Philosopher %d: THINKING\n", id);

    table->completed[id]++;

    delay();
  }

  printf("Philosopher %d: completed all %d cycles\n",
         id, CYCLES);

  exit(0);
}

int
main(void)
{
  struct shared_table *table;
  int pid;
  int children = 0;

  table = (struct shared_table *)shm_get();

  if(table == 0){
    printf("shm_get failed\n");
    exit(1);
  }

  for(int i = 0; i < NUM_PHILOSOPHERS; i++)
    table->completed[i] = 0;

  for(int i = 0; i < NUM_PHILOSOPHERS; i++)
    sem_init(i, 1);

  for(int i = 0; i < NUM_PHILOSOPHERS; i++){
    pid = fork();

    if(pid < 0){
      printf("fork failed\n");
      exit(1);
    }

    if(pid == 0){
      table = (struct shared_table *)shm_get();

      if(table == 0){
        printf("philosopher shm_get failed\n");
        exit(1);
      }

      philosopher(table, i);
    }

    children++;
  }

  for(int i = 0; i < children; i++)
    wait(0);

  printf("Dining Philosophers completed successfully\n");

  for(int i = 0; i < NUM_PHILOSOPHERS; i++)
    printf("Philosopher %d completed %d cycles\n",
           i, table->completed[i]);

  exit(0);
}
