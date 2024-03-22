#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
   program that segfaults, runs an infinite loop, or runs normally
   depending on arguments provided
*/

int main(int argc, char *argv[]) {
  if(argc == 1) {
	fprintf(stderr, "Usage: %s numeric-args\n", argv[0]);
  }

  int i = 1;

  while(strcmp(argv[i], "0") != 0) {
	printf("arg %d: %d\n", i, atoi(argv[i]));
  }

  return 0;
}
