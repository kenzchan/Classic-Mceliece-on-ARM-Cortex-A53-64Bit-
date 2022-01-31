# This Makefile can be used with GNU Make or BSD Make
CC = clang

ASM =

HEADERS = aes.h aes256ctr.h api.h randombytes.h fips202.h crypto_hash.h \
	  pk_gen.h sk_gen.h params.h \
	  decrypt.h encrypt.h \
	  benes.h bm.h controlbits.h \
	  gf.h root.h synd.h transpose.h util.h

SOURCES = aes.c aes256ctr.c randombytes.c fips202.c \
	  operations.c pk_gen.c sk_gen.c \
	  decrypt.c encrypt.c \
	  benes.c bm.c controlbits.c \
	  gf.c root.c synd.c transpose.c util.c

CFLAGS  = -O3 -std=c99 -Wall -Wextra -pedantic -Werror -Wpedantic \
	  -Wredundant-decls -Wvla -Wcast-align -Wmissing-prototypes

all: mceliece348864_test

mceliece348864_test: $(HEADERS) $(ASM) $(SOURCES)  mceliece_test.c
	$(CC) $(CFLAGS) $(ASM) $(SOURCES) -o mceliece348864_test mceliece_test.c

clean:
	rm -f mceliece348864_test

