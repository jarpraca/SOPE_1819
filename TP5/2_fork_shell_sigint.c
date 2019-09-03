#include "common.h"

int main() {
  sigset_t mask;

  set_signal_handler(SIGINT, 0);

  sigfillset(&mask);
  sigdelset(&mask, SIGINT);

  printf("Hit Ctrl+C...\n");
  
  fork_and_set_label();

  print_pending_signals();
  sigsuspend(&mask);

  return 0;
}
