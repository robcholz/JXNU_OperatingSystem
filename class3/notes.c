#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

#define INTERVAL 5

static jmp_buf buf;
static volatile int resumeFlag = 0;
static volatile int exitFlag = 0;

void handler(int sig) {
  if (sig == SIGINT) {
    printf("Received SIGINT. Exiting the program.\n");
    exitFlag = 1;
  } else {
    printf("Interrupted\n");
    resumeFlag = 1;
  }
}

void myFunction() {
  if (!resumeFlag) {
    printf("Function started\n");
    sleep(INTERVAL); // Simulate a long-running task
  } else {
    printf("Function resumed\n");
    resumeFlag = 0;
  }
}

int main() {
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGALRM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);

  while (!exitFlag) {
    alarm(INTERVAL);
    if (setjmp(buf) == 0) {
      myFunction();
      longjmp(buf, 1);
    }
  }

  printf("Exiting the program\n");

  return 0;
}
