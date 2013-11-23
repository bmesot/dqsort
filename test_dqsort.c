#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define elsize 14

#include "dqsort.h"

int compare(const void *e1, const void *e2) {
  return memcmp(e1, e2, elsize);
}

int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 4) {
    printf("Usage: %s <#threads> <input file> <#elements>\n", argv[0]);
    exit(1);
  }

  int ncores = atoi(argv[1]);
  char *input_file = argv[2];

  struct stat file_info;
  stat(input_file, &file_info);

  size_t fsize = file_info.st_size;
  
  assert(fsize % elsize == 0);

  uint64_t nel = fsize / elsize;
  uint64_t n = argc == 4 ? atol(argv[3]) : nel;

  if (n > nel) {
    printf("The file contains less than %ld elements\n", n);
    exit(1);
  }

  printf("File %s contains %ld elements\n", input_file, nel);

  if (n < nel)
    printf("Only the first %ld elements will be sorted\n", n);

  uint8_t *data = (uint8_t*)malloc(fsize);

  FILE *input = fopen(input_file, "r");
  fread(data, elsize, n, input);
  fclose(input);

  printf("Start sort\n");

  time_t before = time(0);

  dqsort(data, n, elsize, compare, ncores);

  time_t after = time(0);

  printf("Sort took %f seconds\n", difftime(after, before));
  printf("Check sort\n");

  for (uint64_t i=0; n>0 && i<n-1; ++i) {
    uint64_t p1 = i*elsize;
    uint64_t p2 = p1 + elsize;

    if (compare(data+p1, data+p2) > 0) {
      char tmp1[elsize+1] = { 0 };
      char tmp2[elsize+1] = { 0 };

      memcpy(tmp1, data+p1, elsize);
      memcpy(tmp2, data+p2, elsize);

      printf("%9ld |%14s| |%14s|\n", i, tmp1, tmp2);
      break;
    }
  }

  /* FILE *output = fopen(output_file, "w"); */
  /* fwrite(data, elsize, n, output); */
  /* fclose(output); */

  free(data);

  return 0;
}
