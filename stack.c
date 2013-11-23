#include <memory.h>
#include <assert.h>

#include "stack.h"

stack_t *stack_create(uint32_t size, uint32_t incr) {
  stack_t *stack = (stack_t*)malloc(sizeof(stack_t));

  sem_init(&stack->count, 0, 0);
  sem_init(&stack->done, 0, 0);

  pthread_mutex_init(&stack->mutex, 0);

  stack->objs    = (void**)calloc(size, sizeof(void*));
  stack->size    = size;
  stack->incr    = incr;
  stack->next    = 0;
  stack->pending = 0;

  return stack;
}

void stack_push(stack_t *stack, void *obj) {
  pthread_mutex_lock(&stack->mutex);

  if (stack->next == stack->size) {
    stack->size += stack->incr;
    stack->objs = (void**)realloc(stack->objs, stack->size*sizeof(void*));
  }

  stack->objs[stack->next] = obj;
  stack->next += 1;

  pthread_mutex_unlock(&stack->mutex);
  sem_post(&stack->count);
}

void *stack_pop(stack_t *stack) {
  sem_wait(&stack->count);
  pthread_mutex_lock(&stack->mutex);
  
  stack->next -= 1;
  stack->pending += 1;

  void *obj = stack->objs[stack->next];

  stack->objs[stack->next] = 0;

  pthread_mutex_unlock(&stack->mutex);

  return obj;
}

void stack_done(stack_t *stack) {
  pthread_mutex_lock(&stack->mutex);
  
  stack->pending -= 1;

  if (stack->next == 0 && stack->pending == 0)
    sem_post(&stack->done);

  pthread_mutex_unlock(&stack->mutex);
}

void stack_wait(stack_t *stack) {
  sem_wait(&stack->done);
}

void stack_free(stack_t *stack) {
  free(stack->objs);
  free(stack);
}
