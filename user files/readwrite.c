#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define RESOURCE 0
#define RC_MUTEX 1
#define TURNSTILE 2

#define NUM_READERS 3
#define NUM_WRITERS 2

#define READER_LOOPS 3
#define WRITER_LOOPS 5

struct shared_data {
  int data;
  int read_count;
};

void
delay(void)
{
  for(volatile int i = 0; i < 300000; i++)
    ;
}

void
reader(struct shared_data *shared, int id)
{
  for(int i = 0; i < READER_LOOPS; i++){

    sem_wait(TURNSTILE);
    sem_signal(TURNSTILE);

    sem_wait(RC_MUTEX);

    shared->read_count++;

    if(shared->read_count == 1)
      sem_wait(RESOURCE);

    printf("Reader %d ENTER, shared_data = %d, readers = %d\n",
           id, shared->data, shared->read_count);

    sem_signal(RC_MUTEX);

    delay();

    sem_wait(RC_MUTEX);

    printf("Reader %d EXIT\n", id);

    shared->read_count--;

    if(shared->read_count == 0)
      sem_signal(RESOURCE);

    sem_signal(RC_MUTEX);

    delay();
  }
}

void
writer(struct shared_data *shared, int id)
{
  for(int i = 0; i < WRITER_LOOPS; i++){

    sem_wait(TURNSTILE);

    sem_wait(RESOURCE);

    shared->data++;

    printf("Writer %d ENTER, shared_data = %d\n",
           id, shared->data);

    delay();

    printf("Writer %d EXIT\n", id);

    sem_signal(RESOURCE);

    sem_signal(TURNSTILE);

    delay();
  }
}

int
main(void)
{
  struct shared_data *shared;
  int pid;
  int children = 0;

  shared = (struct shared_data *)shm_get();

  if(shared == 0){
    printf("shm_get failed\n");
    exit(1);
  }

  shared->data = 0;
  shared->read_count = 0;

  sem_init(RESOURCE, 1);
  sem_init(RC_MUTEX, 1);
  sem_init(TURNSTILE, 1);

  for(int i = 0; i < NUM_READERS; i++){
    pid = fork();

    if(pid < 0){
      printf("fork failed\n");
      exit(1);
    }

    if(pid == 0){
      shared = (struct shared_data *)shm_get();

      if(shared == 0){
        printf("reader shm_get failed\n");
        exit(1);
      }

      reader(shared, i + 1);
      exit(0);
    }

    children++;
  }

  for(int i = 0; i < NUM_WRITERS; i++){
    pid = fork();

    if(pid < 0){
      printf("fork failed\n");
      exit(1);
    }

    if(pid == 0){
      shared = (struct shared_data *)shm_get();

      if(shared == 0){
        printf("writer shm_get failed\n");
        exit(1);
      }

      writer(shared, i + 1);
      exit(0);
    }

    children++;
  }

  for(int i = 0; i < children; i++)
    wait(0);

  printf("Readers-Writers completed successfully\n");
  printf("Final shared_data = %d\n", shared->data);

  exit(0);
}
