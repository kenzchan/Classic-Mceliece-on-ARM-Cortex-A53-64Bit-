CC = gcc

TEST_ITERATIONS ?= 10
SPEED_ITERATIONS ?= 100
KAT_ITERATIONS ?= 30

CFLAGS  = -O3 -march=native -mtune=native \
	  -ftree-vectorize \
	  -Wall -Wextra -Wmissing-prototypes \
	  "-DCRYPTO_NAMESPACE(x) = x"

HEADERs = api.h randombytes.h fips202.h crypto_hash.h \
	  operations.h pk_gen.h sk_gen.h params.h \
	  decrypt.h encrypt.h \
	  benes.h bm.h controlbits.h \
	  gf.h root.h synd.h transpose.h util.h \
	  int32_sort.h uint64_sort.h

SOURCEs = randombytes.c fips202.c \
	  operations.c pk_gen.c sk_gen.c \
	  decrypt.c encrypt.c \
	  benes.c bm.c controlbits.c \
	  gf.c root.c synd.c transpose.c util.c

all: test speed testvectors

test: $(HEADERs) $(SOURCEs) test.c
	$(CC) $(CFLAGS) -o test test.c -DNTESTS=$(TEST_ITERATIONS) -DRANDOM_RANDOMBYTES $(SOURCEs)

speed: $(HEADERs) $(SOURCEs) hal.h speed.c
	$(CC) $(CFLAGS) -o speed hal.c speed.c -DITERATIONS=$(SPEED_ITERATIONS) -DRANDOM_RANDOMBYTES $(SOURCEs)

testvectors: $(HEADERs) $(SOURCEs) testvectors.c
	$(CC) $(CFLAGS) -o testvectors testvectors.c -DKATN=$(KAT_ITERATIONS) $(SOURCEs)

.PHONY: clean
clean:
	rm -f test
	rm -f speed
	rm -f testvectors

