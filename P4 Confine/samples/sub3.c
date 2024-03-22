#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

/*
   program a specified amount of memory
*/

int main(int argc, char *argv[]) {
  if(argc != 2) {
	fprintf(stderr, "Usage: %s allocation-size-in-kb\n", argv[0]);
	exit(1);
  }

  unsigned long size_in_kb;
  char *p;
  size_in_kb = atol(argv[1]);

  printf("Allocating %lu bytes\n", size_in_kb * 1024);

  p = mmap(NULL, size_in_kb * 1024, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if(p == MAP_FAILED) {
	perror("mmap");
	exit(1);
  }

  printf("Allocation succeeded: %p\n", p);

  for(int i = 0; i < (size_in_kb * 1024 / 4096); i++) {
	*p = i;
	p += 4096;
  }

  return 0;
}
