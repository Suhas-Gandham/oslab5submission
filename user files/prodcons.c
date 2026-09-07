#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFFER_SIZE 5

#define EMPTY 0
#define FULL 1
#define MUTEX 2

struct shared_buffer {
  int buffer[BUFFER_SIZE];
  int capacity;
  int in;
  int out;
  int count;
};

void
delay(void)
{
  for(volatile int i = 0; i < 500000; i++)
    ;
}

void
producer(struct shared_buffer *shared)
{
  for(int item = 1; item <= 20; item++){
    if(shared->count == shared->capacity)
      printf("Producer: buffer full, waiting...\n");

    sem_wait(EMPTY);

    sem_wait(MUTEX);

    shared->buffer[shared->in] = item;
    shared->in = (shared->in + 1) % shared->capacity;
    shared->count++;

    printf("Producer: produced %d (buffer count = %d)\n",
           item, shared->count);

    sem_signal(MUTEX);
    sem_signal(FULL);

    delay();
  }
}

void
consumer(struct shared_buffer *shared)
{
  for(int i = 1; i <= 20; i++){
    if(shared->count == 0)
      printf("Consumer: buffer empty, waiting...\n");

    sem_wait(FULL);

    sem_wait(MUTEX);

    int item = shared->buffer[shared->out];

    shared->out = (shared->out + 1) % shared->capacity;
    shared->count--;

    printf("Consumer: consumed %d (buffer count = %d)\n",
           item, shared->count);

    sem_signal(MUTEX);
    sem_signal(EMPTY);

    delay();
  }
}

int
main(int argc, char *argv[])
{
  struct shared_buffer *shared;
  int capacity = BUFFER_SIZE;
  int pid;

  if(argc > 1){
    capacity = atoi(argv[1]);

    if(capacity < 1 || capacity > BUFFER_SIZE){
      printf("Buffer size must be between 1 and %d\n",
             BUFFER_SIZE);
      exit(1);
    }
  }

  shared = (struct shared_buffer *)shm_get();

  if(shared == 0){
    printf("shm_get failed\n");
    exit(1);
  }

  shared->capacity = capacity;
  shared->in = 0;
  shared->out = 0;
  shared->count = 0;

  sem_init(EMPTY, capacity);
  sem_init(FULL, 0);
  sem_init(MUTEX, 1);

  pid = fork();

  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){
    shared = (struct shared_buffer *)shm_get();

    if(shared == 0){
      printf("child shm_get failed\n");
      exit(1);
    }

    consumer(shared);
    exit(0);
  }

  producer(shared);

  wait(0);

  printf("Producer-Consumer completed successfully\n");

  exit(0);
}
