
CC = gcc

TEST_ITERATIONS ?= 10
SPEED_ITERATIONS ?= 1000
KAT_ITERATIONS ?= 10

CFLAGS += -O3 -march=native -mtune=native \
	  -ftree-vectorize \
	  -Wall -Wextra -pedantic -Wpedantic -Wmissing-prototypes \
	  "-DCRYPTO_NAMESPACE(x) = x"

HEADERS = randombytes.h  \
	  params.h api.h benes.h bm.h controlbits.h crypto_int32.h \
	  crypto_kem.h util.h \
	  operations.h pk_gen.h sk_gen.h \
	  decrypt.h encrypt.h \
	  vec.h vec128.h \
	  gf.h fft.h fft_tr.h \
	  int32_sort.h uint16_sort.h uint64_sort.h \
	  fips202.h \
	  crypto_hash.h crypto_int32.h transpose.h

SOURCEs = randombytes.c \
	operations.c pk_gen.c sk_gen.c \
	benes.c bm.c controlbits.c \
	decrypt.c encrypt.c \
	vec.c vec128.c \
	gf.c fft.c fft_tr.c \
	fips202.c

all: test speed testvectors

test: $(HEADERs) $(SOURCEs) test.c
	$(CC) $(CFLAGS) -o test test.c -DNTESTS=$(TEST_ITERATIONS) -DRANDOM_RANDOMBYTES $(SOURCEs)

speed: $(HEADERs) $(SOURCEs) hal.h speed.c
	$(CC) $(CFLAGS) -o speed hal.c speed.c -DITERATIONS=$(SPEED_ITERATIONS) -DRANDOM_RANDOMBYTES $(SOURCEs)

testvectors: $(HEADERs) $(SOURCEs) testvectors.c
	$(CC) $(CFLAGS) -o testvectors testvectors.c -DKAT -DKATN=$(KAT_ITERATIONS) $(SOURCEs)

.PHONY: clean
clean:
	rm -f test
	rm -f speed
	rm -f testvectors




