#include "common.h"

#include <sys/wait.h>

int main() {
  sigset_t mask, omask;

  set_signal_handler(SIGINT, 0);
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);

  sigprocmask(SIG_SETMASK, &mask, &omask);

  print_pending_signals();

  if (fork_and_set_label() == 0) {
  //  setpgid(getpid(), getpid());
    
    sleep(3);
  //  sleep(10);
  }
  else {
    sleep(2);
    
    kill(-getpgid(getpid()), SIGINT);
    kill(-getpgid(getpid()), SIGINT);

    // wait(NULL);
  }

  print_pending_signals();
  sigprocmask(SIG_SETMASK, &omask, NULL);

  return 0;
}
