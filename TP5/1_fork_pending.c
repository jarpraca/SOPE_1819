#include "common.h"

int main() {
  sigset_t mask, omask;

  set_signal_handler(SIGINT, 0);

  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK, &mask, &omask);

  //kill(getpid(), SIGINT);
  kill(getpid(), SIGINT);
  raise(SIGINT);
  //raise(SIGINT);

  print_pending_signals();
  
  fork_and_set_label();

  print_pending_signals();
  sigprocmask(SIG_SETMASK, &omask, NULL);

  return 0;
}
