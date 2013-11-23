#ifndef DQSORT_H
#define DQSORT_H

typedef int cmp_t(const void*, const void*);
void dqsort(void *data, uint64_t n, size_t es, cmp_t *cmp, int nthreads);

#endif
