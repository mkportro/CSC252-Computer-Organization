#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
   program that can hang indefinitely by waiting on stdin
*/

int main(int argc, char *argv[]) {
  if(argc > 2) {
	fprintf(stderr, "Usage: %s file\n", argv[0]);
	exit(1);
  }

  const char *filename = NULL;
  if(argc == 1) {
	filename = "/dev/stdin";
  } else {
	filename = argv[1];
  }

  FILE *f;

  f = fopen(filename, "r");
  if(!f) {
	perror("Failed to open file");
	exit(1);
  }

  int c = 0;
  size_t digits = 0;
  while(!feof(f)) {
	c = fgetc(f);
	if(c == EOF) break;
	if(!(c < '0' || c > '9')) digits++;
  }
  fclose(f);

  printf("Number of digits: %lu\n", digits);
  return 0;
}
