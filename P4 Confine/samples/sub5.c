#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if(argc > 2) {
	fprintf(stderr, "Usage: %s [time]\n", argv[0]);
	exit(1);
  }

  int time = 60;

  if(argc == 2) {
	time = atoi(argv[1]);
  }

  int rem = time;
  do {
	rem = sleep(rem);
  } while(rem > 0);
}
