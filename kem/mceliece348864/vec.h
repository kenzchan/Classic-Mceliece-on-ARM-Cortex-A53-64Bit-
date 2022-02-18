#ifndef VEC_H
#define VEC_H
#define vec_GF_mul CRYPTO_NAMESPACE(vec_GF_mul)
//#define vec_mul_asm CRYPTO_NAMESPACE(vec_mul_asm)

#include "params.h"
#include "gf.h"

#include <stdint.h>
#include <arm_neon.h>
#include <stdio.h>

typedef uint64_t vec;

//extern void vec_mul_asm(vec *, const vec *, const vec *, int);


/*
static inline void vec_mul_64(vec * h, const vec * f, const vec * g)
{
	int i, j;
	uint64_t buf[ 2*GFBITS-1 ];

	for (j = 0; j < GFBITS; j++)
		buf[0+j] = f[0] & g[j];

	for (i = 1; i < GFBITS; i++)
		buf[i+GFBITS-1] = f[i] & g[GFBITS-1];

	for (i = 1; i < GFBITS; i++)
	for (j = 0; j < GFBITS-1; j++)
		buf[i+j] ^= f[i] & g[j];
		
	for (i = 2*GFBITS-2; i >= GFBITS; i--)
	{
		buf[i-GFBITS+3] ^= buf[i]; 
		buf[i-GFBITS+0] ^= buf[i]; 
	}

	for (i = 0; i < GFBITS; i++)
		h[i] = buf[i];
}
*/
/*
static void vec_mul_64(uint64x1_t * h, uint64x1_t * f, uint64x1_t * g)
{
	uint64x1_t buf[ 2*GFBITS-1 ];

	buf[14] = veor_u64(vand_u64(f[11], g[3]), veor_u64(veor_u64(veor_u64(vand_u64(f[10], g[4]), vand_u64(f[9], g[5])),veor_u64(vand_u64(f[8], g[6]), vand_u64(f[7], g[7]))),veor_u64(veor_u64(vand_u64(f[6], g[8]), vand_u64(f[5], g[9])),veor_u64(vand_u64(f[4], g[10]), vand_u64(f[3], g[11])))));
	h[2] = veor_u64(buf[14], veor_u64(vand_u64(f[1], g[1]), veor_u64(vand_u64(f[0], g[2]), vand_u64(f[2], g[0]))));
	h[5] = veor_u64(buf[14], veor_u64(veor_u64(vand_u64(f[0], g[5]), vand_u64(f[5], g[0])), veor_u64(veor_u64(vand_u64(f[4], g[1]), vand_u64(f[1], g[4])), veor_u64(vand_u64(f[3], g[2]), vand_u64(f[2], g[3])))));

	buf[17] = veor_u64(veor_u64(vand_u64(f[11], g[6]), vand_u64(f[10], g[7])), veor_u64(veor_u64(vand_u64(f[9], g[8]), vand_u64(f[8], g[9])), veor_u64(vand_u64(f[7], g[10]), vand_u64(f[6], g[11]))));
	h[5] = veor_u64(h[5], buf[17]);
	h[8] = veor_u64(buf[17], veor_u64(vand_u64(f[4], g[4]), veor_u64(veor_u64(veor_u64(vand_u64(f[0], g[8]), vand_u64(f[8], g[0])),veor_u64(vand_u64(f[7], g[1]), vand_u64(f[1], g[7]))),veor_u64(veor_u64(vand_u64(f[6], g[2]), vand_u64(f[2], g[6])),veor_u64(vand_u64(f[5], g[3]), vand_u64(f[3], g[5]))))));

	buf[20] = veor_u64(vand_u64(f[11], g[9]), veor_u64(vand_u64(f[10], g[10]), vand_u64(f[9], g[11])));
	h[8] = veor_u64(h[8], buf[20]);
	h[11] = veor_u64(buf[20], veor_u64(veor_u64(vand_u64(f[6], g[5]), vand_u64(f[5], g[6])), veor_u64(veor_u64(vand_u64(f[11], g[0]), vand_u64(f[0], g[11])), veor_u64(veor_u64(veor_u64(vand_u64(f[10], g[1]), vand_u64(f[1], g[10])), veor_u64(vand_u64(f[9], g[2]), vand_u64(f[2], g[9]))), veor_u64(veor_u64(vand_u64(f[3], g[8]), vand_u64(f[8], g[3])), veor_u64(vand_u64(f[7], g[4]), vand_u64(f[4], g[7])))))));

	// ok

	buf[18] = veor_u64(vand_u64(f[11], g[7]), veor_u64(veor_u64(vand_u64(f[10], g[8]), vand_u64(f[9], g[9])), veor_u64(vand_u64(f[8], g[10]), vand_u64(f[7], g[11]))));
	h[9] = veor_u64( buf[18], veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[9], g[0]), vand_u64(f[0], g[9])),veor_u64(vand_u64(f[8], g[1]), vand_u64(f[1], g[8]))), veor_u64(veor_u64(vand_u64(f[7], g[2]), vand_u64(f[2], g[7])), veor_u64(vand_u64(f[3], g[6]), vand_u64(f[6], g[3])))), veor_u64(vand_u64(f[4], g[5]), vand_u64(f[5], g[4]))));
	h[6] = veor_u64( buf[18], veor_u64(veor_u64(vand_u64(f[3], g[3]), veor_u64(vand_u64(f[6], g[0]), vand_u64(f[0], g[6]))),veor_u64(veor_u64(vand_u64(f[5], g[1]), vand_u64(f[1], g[5])), veor_u64(vand_u64(f[4], g[2]), vand_u64(f[2], g[4])))));

	buf[21] = veor_u64(vand_u64(f[10], g[11]), vand_u64(f[11], g[10]));
	h[9] = veor_u64(h[9], buf[21]);
	buf[12] = veor_u64(buf[21], veor_u64(vand_u64(f[1], g[11]), veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[1]), vand_u64(f[10], g[2])),veor_u64(vand_u64(f[9], g[3]), vand_u64(f[8], g[4]))), veor_u64(veor_u64(vand_u64(f[7], g[5]), vand_u64(f[6], g[6])), veor_u64(vand_u64(f[5], g[7]), vand_u64(f[4], g[8])))), veor_u64(vand_u64(f[3], g[9]), vand_u64(f[2], g[10])))));

	buf[15] = veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[4]), vand_u64(f[10], g[5])),veor_u64(vand_u64(f[9], g[6]), vand_u64(f[8], g[7]))),veor_u64(veor_u64(vand_u64(f[7], g[8]), vand_u64(f[6], g[9])),veor_u64(vand_u64(f[5], g[10]), vand_u64(f[4], g[11]))));
	h[6] = veor_u64(h[6], buf[15]);
	h[3] = veor_u64(buf[15], veor_u64(veor_u64(vand_u64(f[2], g[1]),vand_u64(f[1], g[2])), veor_u64(vand_u64(f[0], g[3]), vand_u64(f[3], g[0]))));
	h[3] = veor_u64(h[3], buf[12]);
	h[0] = veor_u64(vand_u64(f[0], g[0]), buf[12]);

	//
	buf[19] = veor_u64(veor_u64(vand_u64(f[11], g[8]),vand_u64(f[10], g[9])), veor_u64(vand_u64(f[9], g[10]), vand_u64(f[8], g[11])));
	h[7] = veor_u64(buf[19], veor_u64(veor_u64(veor_u64(vand_u64(f[0], g[7]), vand_u64(f[7], g[0])),veor_u64(vand_u64(f[6], g[1]), vand_u64(f[1], g[6]))),veor_u64(veor_u64(vand_u64(f[5], g[2]), vand_u64(f[2], g[5])),veor_u64(vand_u64(f[4], g[3]), vand_u64(f[3], g[4])))));
	h[10] = veor_u64(buf[19], veor_u64(vand_u64(f[10], g[0]), veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[9], g[1]), vand_u64(f[8], g[2])),veor_u64(vand_u64(f[7], g[3]), vand_u64(f[6], g[4]))), veor_u64(veor_u64(vand_u64(f[5], g[5]), vand_u64(f[4], g[6])), veor_u64(vand_u64(f[3], g[7]), vand_u64(f[2], g[8])))), veor_u64(vand_u64(f[1], g[9]), vand_u64(f[0], g[10])))));

	buf[22] = vand_u64(f[11], g[11]);
	h[10] = veor_u64(h[10], buf[22]);
	buf[13] = veor_u64(buf[22], veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[2]), vand_u64(f[10], g[3])),veor_u64(vand_u64(f[9], g[4]), vand_u64(f[8], g[5]))), veor_u64(veor_u64(vand_u64(f[7], g[6]), vand_u64(f[6], g[7])), veor_u64(vand_u64(f[5], g[8]), vand_u64(f[4], g[9])))), veor_u64(vand_u64(f[3], g[10]), vand_u64(f[2], g[11]))));

	buf[16] = veor_u64(veor_u64(vand_u64(f[11], g[5]), veor_u64(vand_u64(f[10], g[6]), vand_u64(f[9], g[7]))),veor_u64(veor_u64(vand_u64(f[8], g[8]), vand_u64(f[7], g[9])), veor_u64(vand_u64(f[6], g[10]), vand_u64(f[5], g[11]))));
	h[7] = veor_u64(h[7], buf[16]);
	h[4] = veor_u64(buf[16], veor_u64(vand_u64(f[2], g[2]), veor_u64(veor_u64(vand_u64(f[4], g[0]), vand_u64(f[0], g[4])), veor_u64(vand_u64(f[3], g[1]), vand_u64(f[1], g[3])))));
	h[4] = veor_u64(h[4], buf[13]);
	h[1] = veor_u64(buf[13], veor_u64(vand_u64(f[0], g[1]), vand_u64(f[1], g[0])));
}
*/

/*
static void vec_mul_64(uint64x1_t * h, uint64x1_t * f, uint64x1_t * g)
{
	uint64x1_t buf[ 2*GFBITS-1 ];

	uint64x1_t tmp;
	
	buf[0] = vand_u64(f[0], g[0]);

	tmp = vand_u64(f[0], g[1]);
	buf[1] = vand_u64(f[1], g[0]);
	buf[1] = veor_u64(buf[1], tmp);

	tmp = vand_u64(f[1], g[1]);
	buf[2] = vand_u64(f[0], g[2]);
	buf[2] = veor_u64(buf[2], tmp);
	tmp = vand_u64(f[2], g[0]);
	buf[2] = veor_u64(buf[2], tmp);

	tmp = vand_u64(f[2], g[1]);
	buf[3] = vand_u64(f[1], g[2]);
	buf[3] = veor_u64(buf[3], tmp);
	tmp = vand_u64(f[0], g[3]);
	buf[3] = veor_u64(buf[3], tmp);
	tmp = vand_u64(f[3], g[0]);
	buf[3] = veor_u64(buf[3], tmp);

	tmp = vand_u64(f[2], g[2]);
	buf[4] = vand_u64(f[4], g[0]);
	buf[4] = veor_u64(buf[4], tmp);
	tmp = vand_u64(f[0], g[4]);
	buf[4] = veor_u64(buf[4], tmp);
	tmp = vand_u64(f[3], g[1]);
	buf[4] = veor_u64(buf[4], tmp);
	tmp = vand_u64(f[1], g[3]);
	buf[4] = veor_u64(buf[4], tmp);

	tmp = vand_u64(f[0], g[5]);
	buf[5] = vand_u64(f[5], g[0]);
	buf[5] = veor_u64(buf[5], tmp);
	tmp = vand_u64(f[4], g[1]);
	buf[5] = veor_u64(buf[5], tmp);
	tmp = vand_u64(f[1], g[4]);
	buf[5] = veor_u64(buf[5], tmp);
	tmp = vand_u64(f[3], g[2]);
	buf[5] = veor_u64(buf[5], tmp);
	tmp = vand_u64(f[2], g[3]);
	buf[5] = veor_u64(buf[5], tmp);

	tmp = vand_u64(f[3], g[3]);
	buf[6] = vand_u64(f[6], g[0]);
	buf[6] = veor_u64(buf[6], tmp);

	tmp = vand_u64(f[0], g[6]);
	buf[6] = veor_u64(buf[6], tmp);

	tmp = vand_u64(f[5], g[1]);
	buf[6] = veor_u64(buf[6], tmp);

	tmp = vand_u64(f[1], g[5]);
	buf[6] = veor_u64(buf[6], tmp);

	tmp = vand_u64(f[4], g[2]);
	buf[6] = veor_u64(buf[6], tmp);

	tmp = vand_u64(f[2], g[4]);
	buf[6] = veor_u64(buf[6], tmp);

	
	buf[6] = veor_u64(veor_u64(vand_u64(f[3], g[3]), veor_u64(vand_u64(f[6], g[0]), vand_u64(f[0], g[6]))),veor_u64(veor_u64(vand_u64(f[5], g[1]), vand_u64(f[1], g[5])), veor_u64(vand_u64(f[4], g[2]), vand_u64(f[2], g[4]))));
	
	buf[7] = veor_u64(veor_u64(veor_u64(vand_u64(f[0], g[7]), vand_u64(f[7], g[0])),veor_u64(vand_u64(f[6], g[1]), vand_u64(f[1], g[6]))),veor_u64(veor_u64(vand_u64(f[5], g[2]), vand_u64(f[2], g[5])),veor_u64(vand_u64(f[4], g[3]), vand_u64(f[3], g[4]))));

	buf[8] = veor_u64(vand_u64(f[4], g[4]), veor_u64(veor_u64(veor_u64(vand_u64(f[0], g[8]), vand_u64(f[8], g[0])),veor_u64(vand_u64(f[7], g[1]), vand_u64(f[1], g[7]))),veor_u64(veor_u64(vand_u64(f[6], g[2]), vand_u64(f[2], g[6])),veor_u64(vand_u64(f[5], g[3]), vand_u64(f[3], g[5])))));

	buf[9] = veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[9], g[0]), vand_u64(f[0], g[9])),veor_u64(vand_u64(f[8], g[1]), vand_u64(f[1], g[8]))), veor_u64(veor_u64(vand_u64(f[7], g[2]), vand_u64(f[2], g[7])), veor_u64(vand_u64(f[3], g[6]), vand_u64(f[6], g[3])))), veor_u64(vand_u64(f[4], g[5]), vand_u64(f[5], g[4])));

	buf[10] = veor_u64(vand_u64(f[10], g[0]), veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[9], g[1]), vand_u64(f[8], g[2])),veor_u64(vand_u64(f[7], g[3]), vand_u64(f[6], g[4]))), veor_u64(veor_u64(vand_u64(f[5], g[5]), vand_u64(f[4], g[6])), veor_u64(vand_u64(f[3], g[7]), vand_u64(f[2], g[8])))), veor_u64(vand_u64(f[1], g[9]), vand_u64(f[0], g[10]))));

	buf[11] = veor_u64(veor_u64(vand_u64(f[6], g[5]), vand_u64(f[5], g[6])), veor_u64(veor_u64(vand_u64(f[11], g[0]), vand_u64(f[0], g[11])), veor_u64(veor_u64(veor_u64(vand_u64(f[10], g[1]), vand_u64(f[1], g[10])), veor_u64(vand_u64(f[9], g[2]), vand_u64(f[2], g[9]))), veor_u64(veor_u64(vand_u64(f[3], g[8]), vand_u64(f[8], g[3])), veor_u64(vand_u64(f[7], g[4]), vand_u64(f[4], g[7]))))));

	buf[12] = veor_u64(vand_u64(f[1], g[11]), veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[1]), vand_u64(f[10], g[2])),veor_u64(vand_u64(f[9], g[3]), vand_u64(f[8], g[4]))), veor_u64(veor_u64(vand_u64(f[7], g[5]), vand_u64(f[6], g[6])), veor_u64(vand_u64(f[5], g[7]), vand_u64(f[4], g[8])))), veor_u64(vand_u64(f[3], g[9]), vand_u64(f[2], g[10]))));

	buf[13] = veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[2]), vand_u64(f[10], g[3])),veor_u64(vand_u64(f[9], g[4]), vand_u64(f[8], g[5]))), veor_u64(veor_u64(vand_u64(f[7], g[6]), vand_u64(f[6], g[7])), veor_u64(vand_u64(f[5], g[8]), vand_u64(f[4], g[9])))), veor_u64(vand_u64(f[3], g[10]), vand_u64(f[2], g[11])));

	buf[14] = veor_u64(vand_u64(f[11], g[3]), veor_u64(veor_u64(veor_u64(vand_u64(f[10], g[4]), vand_u64(f[9], g[5])),veor_u64(vand_u64(f[8], g[6]), vand_u64(f[7], g[7]))),veor_u64(veor_u64(vand_u64(f[6], g[8]), vand_u64(f[5], g[9])),veor_u64(vand_u64(f[4], g[10]), vand_u64(f[3], g[11])))));

	buf[15] = veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[4]), vand_u64(f[10], g[5])),veor_u64(vand_u64(f[9], g[6]), vand_u64(f[8], g[7]))),veor_u64(veor_u64(vand_u64(f[7], g[8]), vand_u64(f[6], g[9])),veor_u64(vand_u64(f[5], g[10]), vand_u64(f[4], g[11]))));

	buf[16] = veor_u64(veor_u64(vand_u64(f[11], g[5]), veor_u64(vand_u64(f[10], g[6]), vand_u64(f[9], g[7]))),veor_u64(veor_u64(vand_u64(f[8], g[8]), vand_u64(f[7], g[9])), veor_u64(vand_u64(f[6], g[10]), vand_u64(f[5], g[11]))));

	buf[17] = veor_u64(veor_u64(vand_u64(f[11], g[6]), vand_u64(f[10], g[7])), veor_u64(veor_u64(vand_u64(f[9], g[8]), vand_u64(f[8], g[9])), veor_u64(vand_u64(f[7], g[10]), vand_u64(f[6], g[11]))));
	
	buf[18] = veor_u64(vand_u64(f[11], g[7]), veor_u64(veor_u64(vand_u64(f[10], g[8]), vand_u64(f[9], g[9])), veor_u64(vand_u64(f[8], g[10]), vand_u64(f[7], g[11]))));

	buf[19] = veor_u64(veor_u64(vand_u64(f[11], g[8]),vand_u64(f[10], g[9])), veor_u64(vand_u64(f[9], g[10]), vand_u64(f[8], g[11])));

	buf[20] = veor_u64(vand_u64(f[11], g[9]), veor_u64(vand_u64(f[10], g[10]), vand_u64(f[9], g[11])));

	buf[21] = veor_u64(vand_u64(f[10], g[11]), vand_u64(f[11], g[10]));

	buf[22] = vand_u64(f[11], g[11]);

	buf[13] = veor_u64(buf[13], buf[22]); 
	buf[10] = veor_u64(buf[10], buf[22]);

	buf[12] = veor_u64(buf[12], buf[21]); 
	buf[9] = veor_u64(buf[9], buf[21]);

	h[11] = veor_u64(buf[11], buf[20]); 
	buf[8] = veor_u64(buf[8], buf[20]);

	h[10] = veor_u64(buf[10], buf[19]); 
	buf[7] = veor_u64(buf[7], buf[19]);

	h[9] = veor_u64(buf[9], buf[18]); 
	buf[6] = veor_u64(buf[6], buf[18]);

	h[8] = veor_u64(buf[8], buf[17]); 
	buf[5] = veor_u64(buf[5], buf[17]);

	h[7] = veor_u64(buf[7], buf[16]); 
	buf[4] = veor_u64(buf[4], buf[16]);

	h[6] = veor_u64(buf[6], buf[15]); 
	buf[3] = veor_u64(buf[3], buf[15]);

	h[5] = veor_u64(buf[5], buf[14]); 
	h[2] = veor_u64(buf[2], buf[14]);

	h[4] = veor_u64(buf[4], buf[13]); 
	h[1] = veor_u64(buf[1], buf[13]);

	h[3] = veor_u64(buf[3], buf[12]); 
	h[0] = veor_u64(buf[0], buf[12]);
}
*/


static void vec_mul_64(uint64x1_t * h, uint64x1_t * f, uint64x1_t * g)
{
	uint64x1_t buf[ 2*GFBITS-1 ];

	//vec tmp;
	
	buf[0] = vand_u64(f[0], g[0]);

	buf[1] = veor_u64(vand_u64(f[0], g[1]), vand_u64(f[1], g[0]));

	buf[2] = veor_u64(vand_u64(f[1], g[1]), veor_u64(vand_u64(f[0], g[2]), vand_u64(f[2], g[0])));

	buf[3] = veor_u64(veor_u64(vand_u64(f[2], g[1]),vand_u64(f[1], g[2])), veor_u64(vand_u64(f[0], g[3]), vand_u64(f[3], g[0])));

	buf[4] = veor_u64(vand_u64(f[2], g[2]), veor_u64(veor_u64(vand_u64(f[4], g[0]), vand_u64(f[0], g[4])), veor_u64(vand_u64(f[3], g[1]), vand_u64(f[1], g[3]))));

	buf[5] = veor_u64(veor_u64(vand_u64(f[0], g[5]), vand_u64(f[5], g[0])), veor_u64(veor_u64(vand_u64(f[4], g[1]), vand_u64(f[1], g[4])), veor_u64(vand_u64(f[3], g[2]), vand_u64(f[2], g[3]))));
	
	buf[6] = veor_u64(veor_u64(vand_u64(f[3], g[3]), veor_u64(vand_u64(f[6], g[0]), vand_u64(f[0], g[6]))),veor_u64(veor_u64(vand_u64(f[5], g[1]), vand_u64(f[1], g[5])), veor_u64(vand_u64(f[4], g[2]), vand_u64(f[2], g[4]))));
	
	buf[7] = veor_u64(veor_u64(veor_u64(vand_u64(f[0], g[7]), vand_u64(f[7], g[0])),veor_u64(vand_u64(f[6], g[1]), vand_u64(f[1], g[6]))),veor_u64(veor_u64(vand_u64(f[5], g[2]), vand_u64(f[2], g[5])),veor_u64(vand_u64(f[4], g[3]), vand_u64(f[3], g[4]))));

	buf[8] = veor_u64(vand_u64(f[4], g[4]), veor_u64(veor_u64(veor_u64(vand_u64(f[0], g[8]), vand_u64(f[8], g[0])),veor_u64(vand_u64(f[7], g[1]), vand_u64(f[1], g[7]))),veor_u64(veor_u64(vand_u64(f[6], g[2]), vand_u64(f[2], g[6])),veor_u64(vand_u64(f[5], g[3]), vand_u64(f[3], g[5])))));

	buf[9] = veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[9], g[0]), vand_u64(f[0], g[9])),veor_u64(vand_u64(f[8], g[1]), vand_u64(f[1], g[8]))), veor_u64(veor_u64(vand_u64(f[7], g[2]), vand_u64(f[2], g[7])), veor_u64(vand_u64(f[3], g[6]), vand_u64(f[6], g[3])))), veor_u64(vand_u64(f[4], g[5]), vand_u64(f[5], g[4])));

	buf[10] = veor_u64(vand_u64(f[10], g[0]), veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[9], g[1]), vand_u64(f[8], g[2])),veor_u64(vand_u64(f[7], g[3]), vand_u64(f[6], g[4]))), veor_u64(veor_u64(vand_u64(f[5], g[5]), vand_u64(f[4], g[6])), veor_u64(vand_u64(f[3], g[7]), vand_u64(f[2], g[8])))), veor_u64(vand_u64(f[1], g[9]), vand_u64(f[0], g[10]))));

	buf[11] = veor_u64(veor_u64(vand_u64(f[6], g[5]), vand_u64(f[5], g[6])), veor_u64(veor_u64(vand_u64(f[11], g[0]), vand_u64(f[0], g[11])), veor_u64(veor_u64(veor_u64(vand_u64(f[10], g[1]), vand_u64(f[1], g[10])), veor_u64(vand_u64(f[9], g[2]), vand_u64(f[2], g[9]))), veor_u64(veor_u64(vand_u64(f[3], g[8]), vand_u64(f[8], g[3])), veor_u64(vand_u64(f[7], g[4]), vand_u64(f[4], g[7]))))));

	buf[12] = veor_u64(vand_u64(f[1], g[11]), veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[1]), vand_u64(f[10], g[2])),veor_u64(vand_u64(f[9], g[3]), vand_u64(f[8], g[4]))), veor_u64(veor_u64(vand_u64(f[7], g[5]), vand_u64(f[6], g[6])), veor_u64(vand_u64(f[5], g[7]), vand_u64(f[4], g[8])))), veor_u64(vand_u64(f[3], g[9]), vand_u64(f[2], g[10]))));

	buf[13] = veor_u64(veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[2]), vand_u64(f[10], g[3])),veor_u64(vand_u64(f[9], g[4]), vand_u64(f[8], g[5]))), veor_u64(veor_u64(vand_u64(f[7], g[6]), vand_u64(f[6], g[7])), veor_u64(vand_u64(f[5], g[8]), vand_u64(f[4], g[9])))), veor_u64(vand_u64(f[3], g[10]), vand_u64(f[2], g[11])));

	buf[14] = veor_u64(vand_u64(f[11], g[3]), veor_u64(veor_u64(veor_u64(vand_u64(f[10], g[4]), vand_u64(f[9], g[5])),veor_u64(vand_u64(f[8], g[6]), vand_u64(f[7], g[7]))),veor_u64(veor_u64(vand_u64(f[6], g[8]), vand_u64(f[5], g[9])),veor_u64(vand_u64(f[4], g[10]), vand_u64(f[3], g[11])))));

	buf[15] = veor_u64(veor_u64(veor_u64(vand_u64(f[11], g[4]), vand_u64(f[10], g[5])),veor_u64(vand_u64(f[9], g[6]), vand_u64(f[8], g[7]))),veor_u64(veor_u64(vand_u64(f[7], g[8]), vand_u64(f[6], g[9])),veor_u64(vand_u64(f[5], g[10]), vand_u64(f[4], g[11]))));

	buf[16] = veor_u64(veor_u64(vand_u64(f[11], g[5]), veor_u64(vand_u64(f[10], g[6]), vand_u64(f[9], g[7]))),veor_u64(veor_u64(vand_u64(f[8], g[8]), vand_u64(f[7], g[9])), veor_u64(vand_u64(f[6], g[10]), vand_u64(f[5], g[11]))));

	buf[17] = veor_u64(veor_u64(vand_u64(f[11], g[6]), vand_u64(f[10], g[7])), veor_u64(veor_u64(vand_u64(f[9], g[8]), vand_u64(f[8], g[9])), veor_u64(vand_u64(f[7], g[10]), vand_u64(f[6], g[11]))));
	
	buf[18] = veor_u64(vand_u64(f[11], g[7]), veor_u64(veor_u64(vand_u64(f[10], g[8]), vand_u64(f[9], g[9])), veor_u64(vand_u64(f[8], g[10]), vand_u64(f[7], g[11]))));

	buf[19] = veor_u64(veor_u64(vand_u64(f[11], g[8]),vand_u64(f[10], g[9])), veor_u64(vand_u64(f[9], g[10]), vand_u64(f[8], g[11])));

	buf[20] = veor_u64(vand_u64(f[11], g[9]), veor_u64(vand_u64(f[10], g[10]), vand_u64(f[9], g[11])));

	buf[21] = veor_u64(vand_u64(f[10], g[11]), vand_u64(f[11], g[10]));

	buf[22] = vand_u64(f[11], g[11]);

	buf[13] = veor_u64(buf[13], buf[22]); 
	buf[10] = veor_u64(buf[10], buf[22]);

	buf[12] = veor_u64(buf[12], buf[21]); 
	buf[9] = veor_u64(buf[9], buf[21]);

	h[11] = veor_u64(buf[11], buf[20]); 
	buf[8] = veor_u64(buf[8], buf[20]);

	h[10] = veor_u64(buf[10], buf[19]); 
	buf[7] = veor_u64(buf[7], buf[19]);

	h[9] = veor_u64(buf[9], buf[18]); 
	buf[6] = veor_u64(buf[6], buf[18]);

	h[8] = veor_u64(buf[8], buf[17]); 
	buf[5] = veor_u64(buf[5], buf[17]);

	h[7] = veor_u64(buf[7], buf[16]); 
	buf[4] = veor_u64(buf[4], buf[16]);

	h[6] = veor_u64(buf[6], buf[15]); 
	buf[3] = veor_u64(buf[3], buf[15]);

	h[5] = veor_u64(buf[5], buf[14]); 
	h[2] = veor_u64(buf[2], buf[14]);

	h[4] = veor_u64(buf[4], buf[13]); 
	h[1] = veor_u64(buf[1], buf[13]);

	h[3] = veor_u64(buf[3], buf[12]); 
	h[0] = veor_u64(buf[0], buf[12]);
}


static inline void vec_mul(vec * h, const vec * ff, const vec * gg)
{
	int i;
	uint64x1_t h1[GFBITS];
	uint64x1_t f[GFBITS];
	uint64x1_t g[GFBITS];

	for (i = 0; i<GFBITS; i++){
		f[i] = vld1_u64(&ff[i]);
		g[i] = vld1_u64(&gg[i]);
	}

	vec_mul_64(h1, f, g);

 	for (i = 0; i < GFBITS; i++){
 		vst1_u64(&h[i], h1[i]);
 	}
}

static inline void vec_mul_16(vec * h, const vec * f, const vec (*g)[2])
{
	int i, j;
	vec buf[ 2*GFBITS-1 ];

	for (i = 0; i < 2*GFBITS-1; i++)
		buf[i] = 0;

	for (i = 0; i < GFBITS; i++)
	for (j = 0; j < GFBITS; j++)
		buf[i+j] ^= f[i] & g[j][1];
		
	for (i = 2*GFBITS-2; i >= GFBITS; i--)
	{
		buf[i-GFBITS+3] ^= buf[i]; 
		buf[i-GFBITS+0] ^= buf[i]; 
	}

	for (i = 0; i < GFBITS; i++)
		h[i] = buf[i];
}

/*

static inline void vec_mul(vec *h, const vec *f, const vec *g)
{	
	vec_mul_asm(h, f, g, 8);
}
*/

static inline void vec_add(vec *h, vec *f, vec *g) 
{
	int b;

	for (b = 0; b < GFBITS; b++) 
		h[b] = f[b] ^ g[b];
}

static inline void vec_mul_gf(vec out[ GFBITS ], vec v[ GFBITS ], gf a)
{
	int i;

	vec bits[GFBITS];

	for (i = 0; i < GFBITS; i++)
		bits[i] = -((a >> i) & 1);

	vec_mul(out, v, bits);
}

void vec_GF_mul(vec [], vec [], gf []);

#endif

