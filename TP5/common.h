#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <time.h>
#include <unistd.h>

#define PARENT_LABEL "PRNT"
#define CHILD_LABEL "CHLD"

static const int USED_SIGNALS[] = {SIGINT, SIGCHLD, SIGUSR1};
static const char *SIGNAL_STR[] = {
  [SIGINT] = "SIGINT", [SIGCHLD] = "SIGCHLD", [SIGUSR1] = "SIGUSR1"
};
static const char *PROC_LABEL = PARENT_LABEL;

bool is_parent() { return !strcmp(PROC_LABEL, PARENT_LABEL); }

static void sleep_and_print(unsigned int seconds) {
  printf("%s: waited for, at most, %d second(s)\n", PROC_LABEL, seconds - sleep(seconds));
}

static void print_pending_signals() {
  sigset_t mask;

  sigpending(&mask);

  printf("%s: Pending =>", PROC_LABEL);

  for (size_t i = 0; i < sizeof(USED_SIGNALS) / sizeof(USED_SIGNALS[0]); ++i)
    printf(" %s=%d", SIGNAL_STR[USED_SIGNALS[i]], sigismember(&mask, USED_SIGNALS[i]));

  printf("\n");
}

static void sig_handler(int signo) {
  printf("%s: [IN] %s\n", PROC_LABEL, SIGNAL_STR[signo]);

  if (is_parent())
    sleep_and_print(2);

  print_pending_signals();

  //raise(signo);

  printf("%s: [OUT] %s\n", PROC_LABEL, SIGNAL_STR[signo]);
}

void set_signal_handler(int signo, int flags) {
  struct sigaction action;

  action.sa_handler = sig_handler;
  action.sa_flags = flags;

  sigemptyset(&action.sa_mask);

  if(sigaction(signo, &action, NULL) == -1) {
    perror("sigaction");

    exit(1);
  }
}

pid_t fork_and_set_label() {
  pid_t pid;

  switch((pid = fork())) {
    case -1: perror("fork"); exit(2);
    case 0: PROC_LABEL = CHILD_LABEL; break;
    default: break;
  }

  return pid;
}
