CC     := gcc
CFLAGS := -g -Og -Wall -Wextra -Werror

all: main

main: main.c alien.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: all clean
clean:
	rm -f main
