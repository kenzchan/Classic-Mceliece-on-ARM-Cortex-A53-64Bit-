
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "api.h"
#include "crypto_kem.h"

#include "hal.h"

#ifndef ITERATIONS
#define ITERATIONS 100
#endif

uint64_t t0, t1;
uint64_t keypair_times[ITERATIONS];
uint64_t enc_times[ITERATIONS];
uint64_t dec_times[ITERATIONS];

static int cmp_uint64(const void *a, const void *b){
    return (int)((*((const uint64_t*)a)) - (*((const uint64_t*)b)));
}


int main(void){

    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
    uint8_t key_a[CRYPTO_BYTES];
    uint8_t key_b[CRYPTO_BYTES];

    for(size_t i = 0; i < ITERATIONS; i++){

        t0 = hal_get_time();
	crypto_kem_keypair(pk, sk);
	t1 = hal_get_time();
        keypair_times[i] = t1 - t0;

        t0 = hal_get_time();
	crypto_kem_enc(ct, key_b, pk);
	t1 = hal_get_time();
        enc_times[i] = t1 - t0;

        t0 = hal_get_time();
	crypto_kem_dec(key_a, ct, sk);
	t1 = hal_get_time();
        dec_times[i] = t1 - t0;

    }


    qsort(keypair_times, ITERATIONS, sizeof(uint64_t), cmp_uint64);
    qsort(enc_times, ITERATIONS, sizeof(uint64_t), cmp_uint64);
    qsort(dec_times, ITERATIONS, sizeof(uint64_t), cmp_uint64);

    printf("keypair: %ld\n", keypair_times[ITERATIONS >> 1]);
    printf("enc: %ld\n", enc_times[ITERATIONS >> 1]);
    printf("dec: %ld\n", dec_times[ITERATIONS >> 1]);

}


