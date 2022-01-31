#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "randombytes.h"
#include "api.h"

#define NTESTS 10

static int test_keys()
{
  uint8_t pk[PQCLEAN_MCELIECE348864_VEC_CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[PQCLEAN_MCELIECE348864_VEC_CRYPTO_SECRETKEYBYTES];
  uint8_t ct[PQCLEAN_MCELIECE348864_VEC_CRYPTO_CIPHERTEXTBYTES];
  uint8_t key_a[PQCLEAN_MCELIECE348864_VEC_CRYPTO_BYTES];
  uint8_t key_b[PQCLEAN_MCELIECE348864_VEC_CRYPTO_BYTES];

  //Alice generates a public key
  PQCLEAN_MCELIECE348864_VEC_crypto_kem_keypair(pk, sk);

  //Bob derives a secret key and creates a response
  PQCLEAN_MCELIECE348864_VEC_crypto_kem_enc(ct, key_b, pk);

  //Alice uses Bobs response to get her shared key
  PQCLEAN_MCELIECE348864_VEC_crypto_kem_dec(key_a, ct, sk);

  if(memcmp(key_a, key_b, PQCLEAN_MCELIECE348864_VEC_CRYPTO_BYTES)) {
    printf("ERROR keys\n");
    return 1;
  }

  return 0;
}


int main(void)
{
  unsigned int i;
  int r;

  for(i=0;i<NTESTS;i++) {
    r  = test_keys();
    if(r)
      return 1;
  }

  printf("CRYPTO_SECRETKEYBYTES:  %d\n",PQCLEAN_MCELIECE348864_VEC_CRYPTO_SECRETKEYBYTES);
  printf("CRYPTO_PUBLICKEYBYTES:  %d\n",PQCLEAN_MCELIECE348864_VEC_CRYPTO_PUBLICKEYBYTES);
  printf("CRYPTO_CIPHERTEXTBYTES: %d\n",PQCLEAN_MCELIECE348864_VEC_CRYPTO_CIPHERTEXTBYTES);
  printf("Test successful\n");

  return 0;
}





