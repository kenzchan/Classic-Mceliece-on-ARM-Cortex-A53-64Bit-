#ifndef VEC_H
#define VEC_H
#define vec_GF_mul CRYPTO_NAMESPACE(vec_GF_mul)
//#define vec_mul_asm CRYPTO_NAMESPACE(vec_mul_asm)

#include "params.h"
#include "gf.h"

#include <stdint.h>

typedef uint64_t vec;

//extern void vec_mul_asm(vec *, const vec *, const vec *, int);


static inline void vec_mul(vec * h, const vec * f, const vec * g)
{
	int i, j;
	vec buf[ 2*GFBITS-1 ];

	for (i = 0; i < 2*GFBITS-1; i++)
		buf[i] = 0;

	for (i = 0; i < GFBITS; i++)
	for (j = 0; j < GFBITS; j++)
		buf[i+j] ^= f[i] & g[j];
		
	for (i = 2*GFBITS-2; i >= GFBITS; i--)
	{
		buf[i-GFBITS+3] ^= buf[i]; 
		buf[i-GFBITS+0] ^= buf[i]; 
	}

	for (i = 0; i < GFBITS; i++)
		h[i] = buf[i];
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

