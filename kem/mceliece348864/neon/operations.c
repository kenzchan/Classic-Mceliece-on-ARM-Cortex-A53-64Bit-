#include "operations.h"

#include "controlbits.h"
#include "randombytes.h"
#include "crypto_hash.h"
#include "encrypt.h"
#include "decrypt.h"
#include "params.h"
#include "sk_gen.h"
#include "pk_gen.h"
#include "util.h"

#include <stdint.h>
#include <string.h>

/*
extern uint64_t transpose128_time_count;
extern uint64_t transpose128_count;

static inline uint64_t ccnt_read()
{
  uint64_t t = 0;
  asm volatile("mrs %0, PMCCNTR_EL0":"=r"(t));
  return t;
}
*/


int crypto_kem_enc( //524142
       unsigned char *c,
       unsigned char *key,
       const unsigned char *pk
)
{

	unsigned char two_e[ 1 + SYS_N/8 ] = {2};
	unsigned char *e = two_e + 1;
	unsigned char one_ec[ 1 + SYS_N/8 + (SYND_BYTES + 32) ] = {1};

	//
	//uint64_t t0 = ccnt_read();
	encrypt(c, pk, e);

	//uint64_t t1 = ccnt_read();
  	//transpose128_time_count += t1-t0;
  	//transpose128_count += 1;

	//fprintf(stderr, "Error crypto_hash_32b\n");
	crypto_hash_32b(c + SYND_BYTES, two_e, sizeof(two_e)); 

	//fprintf(stderr, "Error memcpy\n");
	memcpy(one_ec + 1, e, SYS_N/8);
	memcpy(one_ec + 1 + SYS_N/8, c, SYND_BYTES + 32);

	//fprintf(stderr, "Error crypto_hash_32b\n");
	crypto_hash_32b(key, one_ec, sizeof(one_ec));
	//fprintf(stderr, "Error crypto_kem_enc\n");

	return 0;
}

int crypto_kem_dec( //1551611
       unsigned char *key,
       const unsigned char *c,
       const unsigned char *sk
)
{
	int i;

	unsigned char ret_confirm = 0;
	unsigned char ret_decrypt = 0;

	uint16_t m;

	unsigned char conf[32];
	unsigned char two_e[ 1 + SYS_N/8 ] = {2};
	unsigned char *e = two_e + 1;
	unsigned char preimage[ 1 + SYS_N/8 + (SYND_BYTES + 32) ];
	unsigned char *x = preimage;
	const unsigned char *s = sk + 40 + IRR_BYTES + COND_BYTES;

	//
	//uint64_t t0 = ccnt_read();

	ret_decrypt = decrypt(e, sk + 40, c);
	//uint64_t t1 = ccnt_read();
  	//transpose128_time_count += t1-t0;
  	//transpose128_count += 1;

	crypto_hash_32b(conf, two_e, sizeof(two_e)); 

	for (i = 0; i < 32; i++) 
		ret_confirm |= conf[i] ^ c[SYND_BYTES + i];

	m = ret_decrypt | ret_confirm;
	m -= 1;
	m >>= 8;

	*x++ = m & 1;
	for (i = 0; i < SYS_N/8; i++) 
		*x++ = (~m & s[i]) | (m & e[i]);

	for (i = 0; i < SYND_BYTES + 32; i++) 
		*x++ = c[i];

	crypto_hash_32b(key, preimage, sizeof(preimage)); 
	//fprintf(stderr, "Error crypto_kem_dec\n");

	return 0;
}

int crypto_kem_keypair //586654955
(
       unsigned char *pk,
       unsigned char *sk 
)
{
	int i;
	unsigned char seed[ 33 ] = {64};
	unsigned char r[ SYS_N/8 + (1 << GFBITS)*sizeof(uint32_t) + SYS_T*2 + 32 ];
	unsigned char *rp, *skp;

	gf f[ SYS_T ]; // element in GF(2^mt)
	gf irr[ SYS_T ]; // Goppa polynomial
	uint32_t perm[ 1 << GFBITS ]; // random permutation as 32-bit integers
	int16_t pi[ 1 << GFBITS ]; // random permutation

	randombytes(seed+1, 32);

	while (1)
	{
		rp = &r[ sizeof(r)-32 ];
		skp = sk;

		// expanding and updating the seed

		shake(r, sizeof(r), seed, 33);
		memcpy(skp, seed+1, 32);
		skp += 32 + 8;
		memcpy(seed+1, &r[ sizeof(r)-32 ], 32);

		// generating irreducible polynomial

		rp -= sizeof(f); 

		for (i = 0; i < SYS_T; i++) 
			f[i] = load_gf(rp + i*2); 

		//uint64_t t0 = ccnt_read();
	
		int ret = genpoly_gen(irr, f);
		
		//uint64_t t1 = ccnt_read();
  		//transpose128_time_count += t1-t0;
  		//transpose128_count += 1;

		if (ret == -1) 
			continue;

		for (i = 0; i < SYS_T; i++)
			store_gf(skp + i*2, irr[i]);

		skp += IRR_BYTES;

		// generating permutation

		rp -= sizeof(perm);

		for (i = 0; i < (1 << GFBITS); i++) 
			perm[i] = load4(rp + i*4); 

		//uint64_t t0 = ccnt_read();
	
		ret = pk_gen(pk, skp - IRR_BYTES, perm, pi);
		
		//uint64_t t1 = ccnt_read();
  		//transpose128_time_count += t1-t0;
  		//transpose128_count += 1;

		if (ret == -1){
			//fprintf(stderr, "Error after genpk\n");
			continue;
		}

		//uint64_t t0 = ccnt_read();

		controlbitsfrompermutation(skp, pi, GFBITS, 1 << GFBITS);

		//uint64_t t1 = ccnt_read();
  		//transpose128_time_count += t1-t0;
  		//transpose128_count += 1;

		skp += COND_BYTES;

		// storing the random string s

		rp -= SYS_N/8;
		memcpy(skp, rp, SYS_N/8);

		// storing positions of the 32 pivots

		store8(sk + 32, 0xFFFFFFFF);
		//fprintf(stderr, "Error crypto_kem_keypair\n");


		break;
	}

	return 0;
}


