#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
  sem_t count;
  sem_t done;
  pthread_mutex_t mutex;
  void **objs;
  uint32_t size;
  uint32_t incr;
  uint32_t next;
  uint32_t pending;
} stack_t;

stack_t *stack_create(uint32_t size, uint32_t incr);
void stack_push(stack_t *stack, void *obj);
void *stack_pop(stack_t *stack);
void stack_done(stack_t *stack);
void stack_wait(stack_t *stack);
void stack_free(stack_t *stack);

#endif
