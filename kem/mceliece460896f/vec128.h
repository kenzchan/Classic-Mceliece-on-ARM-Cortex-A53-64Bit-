/*
  This file is for functions related to 128-bit vectors
  including functions for bitsliced field operations
*/

#ifndef VEC128_H
#define VEC128_H
#define vec128_inv CRYPTO_NAMESPACE(vec128_inv)
#define vec128_mul_asm CRYPTO_NAMESPACE(vec128_mul_asm)
#define vec128_mul_GF CRYPTO_NAMESPACE(vec128_mul_GF)
#define vec128_sq CRYPTO_NAMESPACE(vec128_sq)

#include "params.h"
#include "gf.h"

#include <stdint.h>
#include <arm_neon.h>
#include <stdio.h>

typedef uint64x2_t vec128;
/*
extern uint32_t transpose128_time_count;
extern uint32_t transpose128_count;


static inline uint32_t ccnt_read (void)
{
  uint32_t cc = 0;
  __asm__ volatile ("mrc p15, 0, %0, c9, c13, 0":"=r" (cc));
  return cc;
}
*/

static inline vec128 vec128_set1_16b(uint16_t a)
{
	return vreinterpretq_u64_u16(vmovq_n_u16(a));
}

static inline vec128 vec128_set1_32b(uint64_t a)
{
	return vreinterpretq_u64_u32(vmovq_n_u32 (a));
}

static inline vec128 vec128_setzero() 
{
	vec128 ret = {0, 0};
	return ret;
}

//#define vec128_extract(a, i) ((uint64_t) _mm_extract_epi64((vec128) (a), (i)))
static inline uint64_t vec128_extract(vec128 a, int s) {
	//fprintf(stderr, "Error vec128_extract\n");
	return a[s];
}

static inline int vec128_testz(vec128 a) 
{
	if ((a[0] != 0) || (a[1] != 0)) return 0;
	return 1;
}

static inline vec128 vec128_and(vec128 a, vec128 b) 
{
	return vandq_u64(a, b);
}

static inline vec128 vec128_xor(vec128 a, vec128 b) 
{
	return veorq_u64(a, b);
}

static inline vec128 vec128_or(vec128 a, vec128 b) 
{
	return vorrq_u64(a, b);
}

//#define vec128_sll_2x(a, s) ((vec128) _mm_slli_epi64((vec128) (a), (s)))
//#define vec128_srl_2x(a, s) ((vec128) _mm_srli_epi64((vec128) (a), (s)))
static inline vec128 vec128_sll_2x(vec128 a, int s) {
	//fprintf(stderr, "Error vec128_sll_2x\n");

	vec128 ret = {a[0]<<s, a[1]<<s};
	return ret;
}

//#define vec128_srl_2x(a, s) ((vec128) _mm_srli_epi64((vec128) (a), (s)))
static inline vec128 vec128_srl_2x(vec128 a, int s) {
	//fprintf(stderr, "Error vec128_srl_2x\n");

	uint64_t a0 = a[0];
	uint64_t a1 = a[1];
	vec128 ret = { a0>>s, a1>>s};
	return ret;
	//return _mm_srli_epi64( a, s);
}


static inline vec128 vec128_set2x(uint64_t a0, uint64_t a1)
{
	vec128 ret = { a0, a1};
	return ret;
}

static inline vec128 vec128_unpack_low(vec128 a, vec128 b)
{
	vec128 ret = {a[0], b[0]};
	return ret;
}

static inline vec128 vec128_unpack_high(vec128 a, vec128 b)
{
	vec128 ret = {a[1], b[1]};
	return ret;
}

static inline vec128 vec128_setbits(uint64_t a)
{
	vec128 ret = {-a, -a};
	return ret;
}

static inline void vec128_copy(vec128 *dest, vec128 *src)
{
	int i;

	for (i = 0; i < GFBITS; i++)
		dest[i] = src[i];
}

static inline void vec128_add(vec128 *c, vec128 *a, vec128 *b)
{
	int i;

	for (i = 0; i < GFBITS; i++)
		c[i] = vec128_xor(a[i], b[i]);
}

static inline vec128 vec128_or_reduce(vec128 * a) 
{
	int i;
	vec128 ret;		

	ret = a[0];
	for (i = 1; i < GFBITS; i++)
		ret = vec128_or(ret, a[i]);

	return ret;
}

//extern void vec128_mul_asm(vec128 *, vec128 *, const vec128 *);

/* bitsliced field multiplications */
static inline void vec128_mul(vec128 *h, vec128 *f, const vec128 *g) // 1218
{
	int i, j;
	vec128 buf[ 2*GFBITS-1 ];

	for (i = 0; i < 2*GFBITS-1; i++){
		buf[i][0] = 0;
		buf[i][1] = 0;
	}	

	for (i = 0; i < GFBITS; i++)
	for (j = 0; j < GFBITS; j++)
		buf[i+j] ^= f[i] & g[j];
		
	for (i = 2*GFBITS-2; i >= GFBITS; i--)
	{
		buf[i-GFBITS+4] ^= buf[i]; 
		buf[i-GFBITS+3] ^= buf[i]; 
		buf[i-GFBITS+1] ^= buf[i]; 
		buf[i-GFBITS+0] ^= buf[i]; 
	}

	for (i = 0; i < GFBITS; i++)
		h[i] = buf[i];
}

void vec128_sq(vec128 *, vec128 *);
void vec128_inv(vec128 *, vec128 *);

static inline void vec128_mul_gf(vec128 out[ GFBITS ], vec128 v[ GFBITS ], gf a)
{
	int i;

	uint64_t b; 
	vec128 bits[GFBITS];

	for (i = 0; i < GFBITS; i++)
	{
		b = -((a >> i) & 1);
		bits[i] = vec128_set2x(b, b);
	}

	vec128_mul(out, v, bits);
}

static inline void vec_mul_gf(uint64_t out[ GFBITS ], uint64_t v[ GFBITS ], gf a)
{
	int i;

	uint64_t bits[GFBITS];
	vec128 x[GFBITS], y[GFBITS], z[GFBITS];

	for (i = 0; i < GFBITS; i++)
		bits[i] = -((a >> i) & 1);

	for (i = 0; i < GFBITS; i++)
	{
		x[i] = vec128_set2x(v[i], 0);
		y[i] = vec128_set2x(bits[i], 0);
	}

	vec128_mul(z, x, y);

	for (i = 0; i < GFBITS; i++)
		out[i] = vec128_extract(z[i], 0);
}

void vec128_mul_GF(vec128 [], vec128 [], gf []);

#endif

