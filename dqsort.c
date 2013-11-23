#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <math.h>

#include "stack.h"
#include "dqsort.h"

#define min(a, b) (a) < (b) ? a : b
#define max(a, b) (a) > (b) ? a : b

typedef struct {
  int id;
  stack_t *stack;
} worker_t;

typedef struct {
  uint8_t *data;
  uint64_t n;
  size_t es;
  cmp_t *cmp;
} job_t;

uint8_t *med3(uint8_t *a, uint8_t *b, uint8_t *c, cmp_t *cmp) {
  return cmp(a, b) < 0 ? 
    (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a )) :
    (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ));
}

void swap(uint8_t *a, uint8_t *b, uint8_t *swapbuf, size_t es, uint64_t n) {
  for (uint64_t i=0; i<n; ++i, a += es, b += es) {
    memcpy(swapbuf, a, es);
    memcpy(a, b, es);
    memcpy(b, swapbuf, es);
  }
}

void sort(stack_t *stack, job_t *job, uint8_t *swapbuf) {  
  uint8_t *a = job->data;
  uint64_t n = job->n;
  size_t es  = job->es;
  cmp_t *cmp = job->cmp;

  if (n < 10000) {
    qsort(a, n, es, cmp);
    free(job);
    return;
  }

  uint8_t *pl = a;
  uint8_t *pm = a + n / 2 * es;
  uint8_t *pn = a + (n - 1) * es;
  uint64_t d  = n / 8 * es;
  
  pl = med3(pl, pl + d, pl + 2 * d, cmp);
  pm = med3(pm - d, pm, pm + d, cmp);
  pn = med3(pn - 2 * d, pn - d, pn, cmp);
  pm = med3(pl, pm, pn, cmp);

  swap(a, pm, swapbuf, es, 1);

  uint8_t *pa = a + es;
  uint8_t *pb = pa;
  uint8_t *pc = a + (n - 1) * es;
  uint8_t *pd = pc;

  while (1) {
    for (int r = cmp(pb, a); pb <= pc && r <= 0; pb += es, r = cmp(pb, a)) {
      if (r == 0) {
	swap(pa, pb, swapbuf, es, 1);
	pa += es;
      }
    }
    
    for (int r = cmp(pc, a); pb <= pc && r >= 0; pc -= es, r = cmp(pc, a)) {
      if (r == 0) {
	swap(pc, pd, swapbuf, es, 1);
	pd -= es;
      }
    }

    if (pb > pc)
      break;

    swap(pb, pc, swapbuf, es, 1);

    pb += es;
    pc -= es;
  }

  pn = a + n * es;

  size_t ra = min(pa - a, pb - pa);
  size_t rb = min(pd - pc, pn - pd - es);

  swap(a, pb - ra, swapbuf, es, ra / es);
  swap(pb, pn - rb, swapbuf, es, rb / es);

  uint64_t nl = (pb - pa) / es;
  uint64_t nr = (pd - pc) / es;

  job->n = nl;
  stack_push(stack, job);

  job = (job_t*)malloc(sizeof(job_t));

  job->data = pn - nr * es;
  job->n    = nr;
  job->es   = es;
  job->cmp  = cmp;

  stack_push(stack, job);
}

void *worker_fun(void *arg) {
  worker_t *self   = (worker_t*)arg;
  stack_t *stack   = self->stack;
  uint8_t *swapbuf = 0;

  while (1) {
    job_t *job = (job_t*)stack_pop(stack);

    if (job == 0)
      break;

    if (swapbuf == 0)
      swapbuf = (uint8_t*)malloc(job->es);

    sort(stack, job, swapbuf);
    stack_done(stack);
  }

  if (swapbuf != 0)
    free(swapbuf);

  return 0;
}

void dqsort(void *data, uint64_t n, size_t es, cmp_t *cmp, int nthreads) {
  if (n == 0) return;

  uint32_t ssize = (uint32_t)ceil(nthreads * log2(n) + nthreads);
  stack_t *stack = stack_create(ssize, ssize/2);

  pthread_t *threads = (pthread_t*)malloc(nthreads*sizeof(pthread_t));
  worker_t *workers = (worker_t*)malloc(nthreads*sizeof(worker_t));

  for (int i=0; i<nthreads; ++i) {
    workers[i].id = i;
    workers[i].stack = stack;

    pthread_create(threads+i, 0, worker_fun, workers+i);
  }

  // Sort the whole list

  job_t *job = (job_t*)malloc(sizeof(job_t));

  job->data = data;
  job->n    = n;
  job->es   = es;
  job->cmp  = cmp;

  stack_push(stack, job);

  // Stop the workers

  stack_wait(stack);

  for (int i=0; i<nthreads; ++i)
    stack_push(stack, 0);

  // Wait until all workers are done with their jobs

  for (int i=0; i<nthreads; ++i)
    pthread_join(threads[i], 0);

  stack_free(stack);
}
