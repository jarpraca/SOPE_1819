#include "common.h"

#include <sys/wait.h>

int main(int argc, char *argv[]) {
  sigset_t mask, omask;
  int status = 0, ret = 0;

  if (argc == 1) {
    set_signal_handler(SIGINT, 0);

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    sigprocmask(SIG_SETMASK, &mask, &omask);

    print_pending_signals();

    switch (fork_and_set_label()) {
      case -1: perror("fork"); return 2;
      case 0: execlp(argv[0], argv[0], "dummy", NULL); return 4;
      default: break;
    }

    sleep(1);
    kill(-getpgid(getpid()), SIGINT);
  }
  else {
    set_signal_handler(SIGINT,0);
    PROC_LABEL = CHILD_LABEL;
    sleep(2);
    print_pending_signals();
    sigemptyset(&omask);
    (void) argv;
  }

  print_pending_signals();
  sigprocmask(SIG_SETMASK, &omask, NULL);

  if (is_parent()) {
    ret = wait(&status);

    if (WIFEXITED(status))
      printf("WAIT: %d => %d\n", ret, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
      printf("WAIT (SIGNALED): %d => %d\n", ret, status);
    else
      printf("WAIT (OTHER): %d => %d\n", ret, status);
  }

  return 0;
}
