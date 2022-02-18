#ifndef crypto_kem_H
#define crypto_kem_H


#ifdef __cplusplus
extern "C" {
#endif
extern int crypto_kem_mceliece348864_sse_keypair(unsigned char *,unsigned char *);
extern int crypto_kem_mceliece348864_sse_enc(unsigned char *,unsigned char *,const unsigned char *);
extern int crypto_kem_mceliece348864_sse_dec(unsigned char *,const unsigned char *,const unsigned char *);
#ifdef __cplusplus
}
#endif

#define crypto_kem_keypair crypto_kem_mceliece348864_sse_keypair
#define crypto_kem_enc crypto_kem_mceliece348864_sse_enc
#define crypto_kem_dec crypto_kem_mceliece348864_sse_dec
#define crypto_kem_PUBLICKEYBYTES crypto_kem_mceliece348864_sse_PUBLICKEYBYTES
#define crypto_kem_SECRETKEYBYTES crypto_kem_mceliece348864_sse_SECRETKEYBYTES
#define crypto_kem_BYTES crypto_kem_mceliece348864_sse_BYTES
#define crypto_kem_CIPHERTEXTBYTES crypto_kem_mceliece348864_sse_CIPHERTEXTBYTES
#define crypto_kem_PRIMITIVE "mceliece348864"

#endif
