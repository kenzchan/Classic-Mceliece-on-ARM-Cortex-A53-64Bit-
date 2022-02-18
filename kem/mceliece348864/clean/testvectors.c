/*
   PQCgenKAT_kem.c
   Created by Bassham, Lawrence E (Fed) on 8/29/17.
   Copyright © 2017 Bassham, Lawrence E (Fed). All rights reserved.
   + mods from djb: see KATNOTES
*/

// don't use this file now, still building

#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api.h"

#ifndef KATN
#define KATN 10
#endif

static void printbytes(const uint8_t *x, size_t xlen) {
    size_t i;
    for (i = 0; i < xlen; i++) {
        printf("%02x", x[i]);
    }
    printf("\n");
}

int main(void){

    uint8_t pk[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES];
    uint8_t ct[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_CIPHERTEXTBYTES];
    uint8_t key_a[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_BYTES];
    uint8_t key_b[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_BYTES];

    for(size_t i = 0; i < KATN; i++){

        PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_keypair(pk, sk);
        printbytes(pk, PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES);
        printbytes(sk, PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES);

        PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_enc(ct, key_b, pk);
        printbytes(ct, PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_CIPHERTEXTBYTES);
        printbytes(key_b, PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_BYTES);

	PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_dec(key_a, ct, sk);
        printbytes(key_a, PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_BYTES);

    }

}








