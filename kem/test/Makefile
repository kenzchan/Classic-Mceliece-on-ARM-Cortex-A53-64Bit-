
all: test

CFLAGS += -O3 -mcpu=native -mtune=native -march=native -ftree-vectorize \
	  -Wall -Wextra -Wpedantic -Wredundant-decls -Wmissing-prototypes

test: test.c hal.h
	gcc $(CFLAGS) test.c hal.c -o test

.PHONY: clean

clean:
	rm -f test


