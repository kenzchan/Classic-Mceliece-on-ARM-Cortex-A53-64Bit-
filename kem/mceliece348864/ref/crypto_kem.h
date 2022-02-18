#ifndef crypto_kem_H
#define crypto_kem_H


#ifdef __cplusplus
extern "C" {
#endif
extern int crypto_kem_mceliece348864_ref_keypair(unsigned char *,unsigned char *);
extern int crypto_kem_mceliece348864_ref_enc(unsigned char *,unsigned char *,const unsigned char *);
extern int crypto_kem_mceliece348864_ref_dec(unsigned char *,const unsigned char *,const unsigned char *);
#ifdef __cplusplus
}
#endif


#define crypto_kem_keypair crypto_kem_mceliece348864_ref_keypair
#define crypto_kem_enc crypto_kem_mceliece348864_ref_enc
#define crypto_kem_dec crypto_kem_mceliece348864_ref_dec
#define CRYPTO_PRIMITIVE "mceliece348864"

#endif
