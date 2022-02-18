CC = gcc

ASM =

HEADERs = aes.h aes256ctr.h api.h randombytes.h fips202.h crypto_hash.h \
	  pk_gen.h sk_gen.h params.h \
	  decrypt.h encrypt.h \
	  benes.h bm.h controlbits.h \
	  gf.h root.h synd.h transpose.h util.h

SOURCEs = aes.c aes256ctr.c randombytes.c fips202.c \
	  operations.c pk_gen.c sk_gen.c \
	  decrypt.c encrypt.c \
	  benes.c bm.c controlbits.c \
	  gf.c root.c synd.c transpose.c util.c

CFLAGS  = -O3 -march=native -mtune=native -Wall -Wextra -pedantic -Werror -Wpedantic \
	  -Wredundant-decls -Wvla -Wcast-align -Wmissing-prototypes

all: test testvectors

test: $(HEADERs) $(ASM) $(SOURCEs) test.c
	$(CC) $(CFLAGS) $(ASM) $(SOURCEs) -o test -DRANDOM_RANDOMBYTES test.c

testvectors: $(HEADERs) $(ASM) $(SOURCEs) testvectors.c
	$(CC) $(CFLAGS) $(ASM) $(SOURCEs) -o testvectors testvectors.c

.PHONY: clean
clean:
	rm -f test
	rm -f testvectors

