CC = gcc

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

CFLAGS  = -O3 -march=native -mtune=native \
	  "-DCRYPTO_NAMESPACE(x) = x" \
	  -Wall -Wextra -pedantic -Wpedantic \
	  -Wmissing-prototypes

all: test testvectors

test: $(HEADERs) $(SOURCEs) test.c
	$(CC) $(CFLAGS) $(SOURCEs) -o test -DRANDOM_RANDOMBYTES test.c

testvectors: $(HEADERs) $(SOURCEs) testvectors.c
	$(CC) $(CFLAGS) $(SOURCEs) -o testvectors testvectors.c

.PHONY: clean
clean:
	rm -f test
	rm -f testvectors

