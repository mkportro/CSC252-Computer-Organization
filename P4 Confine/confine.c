#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

//void sig_handler(int sig);

pid_t PID;
//int success = 0; //65< x >0 if child succeeds
//int sigNum = 0;
//int timeout = 0;

/*
void sig_handler(int sig) {
  //PPID = getppid();
  timeout++;
  kill(PID, SIGKILL);
}*/

/* An example of a call to confine will look like this: ./confine ./samples/sub1 1 2 3 0
   This will cause confine to execute ./samples/sub1 with the arguments 1 2 3 0. */

/* alarm & signal handle before fork; fork; set limits in child; call exec w/ argument;
    signal = failure; exit; go to parent and handle fail/succeed */

int main(int argc, char *argv[]) {
  char *newargv[argc-1];
  char *newenviron[] = {NULL};
  FILE *f = fopen("confine_result.txt", "w+");
  struct rlimit mem_limit, file_limit, time_limit;

  //copy parameters from input
  for (int i = 1; i < argc; i++) {
    newargv[i-1] = argv[i];
  }
  newargv[argc-1] = NULL;

  if(argc == 1) {
	fprintf(stderr, "Usage: %s /path/to/program args...\n", argv[0]);
	exit(1);
  }

  //create file confine_result.txt
  //  line 1 - confined program and args
  //  line 2 - "normal" "terminated" or "timeout"
  for (int i = 1; i < argc; i++) {
      fputs(argv[i], f);
      fputs(" ", f);
  }
  fputs("\n", f);
  
  //signal handling
  //struct sigaction sa;
  //sa.sa_handler = &sig_handler;
  //alarm - "this set up should be done in the parent, before fork()"

  //fork
  if ((PID = fork()) == 0) { /* CHILD */
    //set limits - "call setrlimit in the child process after fork() but before execve()"
    //getrlimit(RLIMIT_CPU, time_limit); //60 sec
    getrlimit(RLIMIT_AS, &mem_limit); //memory <= 64MB (64x1024x1024 bytes)
    getrlimit(RLIMIT_FSIZE, &file_limit); //file size <= 4MB (4x1024x1024 bytes)
    //time_limit->rlim_cur = time_limit->rlim_max = 60;
    mem_limit.rlim_cur = mem_limit.rlim_max = 64*1024*1024;
    file_limit.rlim_cur = file_limit.rlim_max = 4*1024*1024;
    //setrlimit(RLIMIT_CPU, time_limit);
    setrlimit(RLIMIT_AS, &mem_limit);
    setrlimit(RLIMIT_FSIZE, &file_limit);
    
    alarm(60);
    execve(argv[1], newargv, newenviron);
    perror("execve");
    exit(EXIT_FAILURE);
  } else { /* PARENT */
    int child_status;
    wait(&child_status);
    //sigaction(SIGALRM, &sa, NULL);
    
    //check child return normal, then signal received
    if (WIFEXITED(child_status)) { //returns true if the child terminated normally
      fputs("NORMAL\n", f);
      fclose(f);
      exit(WEXITSTATUS(child_status));
      //return WEXITSTATUS(child_status);
      /*
      if (WEXITSTATUS(child_status) >= 0) { //normal
      }*/
      /*
      else { //bug
        fputs("TERMINATED", f);
        fclose(f);
        return 128;
      }
      */
    } else if (WIFSIGNALED(child_status)) { //returns true if the child process was terminated by a signal
      if (WTERMSIG(child_status) == SIGALRM) { //time
        kill(PID, SIGKILL);
        fputs("TIMEOUT\n", f);
        fclose(f);
        exit(127);
        //return 127;
      } else /*if (WTERMSIG(child_status) == SIGSEGV || WTERMSIG(child_status) == SIGXFSZ)*/ { //mem or file
        fputs("TERMINATED\n", f);
        fclose(f);
        exit(128);
        //return 128;
      }
    }
  }
}