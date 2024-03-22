#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

/*
   program that creates a file of a specified size
*/

int main(int argc, char *argv[]) {
  if(argc != 3) {
	fprintf(stderr, "Usage: %s file write_size_in_kb\n", argv[0]);
	exit(1);
  }

  unsigned long size_in_kb;
  char *filename;
  filename = argv[1];
  size_in_kb = atol(argv[2]);

  FILE *f;
  f = fopen(filename, "w");

  for(int i = 0; i < (size_in_kb*1024); i++) {
	if(fputc('A', f) == EOF) {
	  perror("fputc");
	  exit(1);
	}
  }

  fclose(f);

  return 0;
}
