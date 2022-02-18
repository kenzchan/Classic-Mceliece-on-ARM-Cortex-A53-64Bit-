
CC = gcc

CFLAGS += -O3 -march=native -mtune=native \
	  -ftree-vectorize -Wall \
	  -DKAT -DKATNUM=10 "-DCRYPTO_NAMESPACE(x) = x"

HEADERS = kat_rng.h randombytes.h  \
	  params.h api.h benes.h bm.h controlbits.h crypto_int32.h \
	  crypto_kem.h crypto_kem_mceliece348864.h util.h \
	  operations.h pk_gen.h sk_gen.h \
	  decrypt.h encrypt.h \
	  vec.h vec128.h \
	  gf.h fft.h fft_tr.h \
	  int32_sort.h uint16_sort.h uint64_sort.h \
	  fips202.h \
	  crypto_hash.h crypto_int32.h transpose.h

SOURCEs = kat_kem.c rng.c \
	operations.c pk_gen.c sk_gen.c \
	benes.c bm.c controlbits.c \
	decrypt.c encrypt.c \
	vec.c vec128.c \
	gf.c fft.c fft_tr.c \
	fips202.c

kat: $(HEADERs) $(SOURCEs)
	$(CC) $(CFLAGS) -o kat $(SOURCEs)

