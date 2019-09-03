CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -D_DEFAULT_SOURCE
DEPS = auxiliar.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

forensic: forensic.o auxiliar.o
	$(CC) -o forensic forensic.o auxiliar.o

.PHONY: all clean

clean:
	rm -f $(TARGETS) *.o *.d *.i *.s