objects= server user
all:$(objects)
CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -D_DEFAULT_SOURCE 
DEPS = 

%.objects: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: server.o 
	$(CC) -o server server.o -lrt -lpthread
user: user.o
	$(CC) -o user user.o


.PHONY: all clean

clean:
	rm -f $(TARGETS) *.o *.d *.i *.s
