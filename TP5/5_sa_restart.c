#include "common.h"

#define BUF_SIZE 512

int main(void) {
  char buf[BUF_SIZE];

  set_signal_handler(SIGINT, 0);
  set_signal_handler(SIGINT, SA_RESTART);

  print_pending_signals();

  ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));

  printf("N: %ld, errno=%d [%s]\n", n, errno, strerror(errno));

  return 0;
}
