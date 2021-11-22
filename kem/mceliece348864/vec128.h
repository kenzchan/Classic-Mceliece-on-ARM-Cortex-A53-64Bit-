/*
  This file is for functions related to 128-bit vectors
  including functions for bitsliced field operations
*/

#ifndef VEC128_H
#define VEC128_H
#define vec128_inv CRYPTO_NAMESPACE(vec128_inv)
//#define vec128_mul_asm CRYPTO_NAMESPACE(vec128_mul_asm)
#define vec128_mul_GF CRYPTO_NAMESPACE(vec128_mul_GF)
#define vec128_sq CRYPTO_NAMESPACE(vec128_sq)

#include "params.h"

#include <stdint.h>
#include <arm_neon.h>

#include <stdio.h>

//#include <smmintrin.h>

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
	//fprintf(stderr, "Error vec128_setzero\n");
	vec128 ret = {0, 0};
	return ret;
	
	//return _mm_setzero_si128();
}

//#define vec128_extract(a, i) ((uint64_t) _mm_extract_epi64((vec128) (a), (i)))
static inline uint64_t vec128_extract(vec128 a, int s) {
	//fprintf(stderr, "Error vec128_extract\n");
	return a[s];
}

static inline int vec128_testz(vec128 a) 
{
	uint32x4_t zeros = {0, 0, 0, 0};

	uint32x4_t tmp = vtstq_u32(vreinterpretq_u32_u64(a), zeros);

	for (int j = 0; j < 4; j++){
		if (tmp[j] != 0) {
			return 0;
		} 
	}
	return 1;

}

static inline vec128 vec128_and(vec128 a, vec128 b) 
{
	//fprintf(stderr, "Error vec128_and\n");
	return vandq_u64(a, b);
	//return _mm_and_si128(a, b);
}

static inline vec128 vec128_xor(vec128 a, vec128 b) 
{
	return veorq_u64(a, b);
	//return a^b;
	//return _mm_xor_si128(a, b);
}

static inline vec128 vec128_or(vec128 a, vec128 b) 
{
	//fprintf(stderr, "Error vec128_or\n");
	return vorrq_u64(a, b);
	//return _mm_or_si128(a, b);
}

//#define vec128_sll_2x(a, s) ((vec128) _mm_slli_epi64((vec128) (a), (s)))
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
	//fprintf(stderr, "Error vec128_set2x\n");

	vec128 ret = { a0, a1};
	return ret;
	//return _mm_set_epi64x(a1, a0);
}

static inline vec128 vec128_unpack_low(vec128 a, vec128 b)
{
	//fprintf(stderr, "Error vec128_unpack_low\n");

	vec128 ret = {a[0], b[0]};
	return ret;
	//return _mm_unpacklo_epi64(a, b);
}

static inline vec128 vec128_unpack_high(vec128 a, vec128 b)
{
	//fprintf(stderr, "Error vec128_unpack_high\n");

	vec128 ret = {a[1], b[1]};
	return ret;
	//return _mm_unpackhi_epi64(a, b);
}

static inline vec128 vec128_setbits(uint64_t a)
{
	//fprintf(stderr, "Error vec128_setbits\n");

	vec128 ret = {-a, -a};
	return ret;
	//return _mm_set1_epi64x(-a);
	//int64x2_t a = vdupq_n_s64(-a);
}

static inline void vec128_copy(vec128 *dest, vec128 *src)
{
	//fprintf(stderr, "Error vec128_copy\n");

	int i;

	for (i = 0; i < GFBITS; i++)
		dest[i] = src[i];
}

static inline void vec128_add(vec128 *c, vec128 *a, vec128 *b)
{
	//fprintf(stderr, "Error vec128_add\n");

	int i;

	for (i = 0; i < GFBITS; i++)
		c[i] = vec128_xor(a[i], b[i]);
}

static inline vec128 vec128_or_reduce(vec128 * a) 
{
	//fprintf(stderr, "Error vec128_or_reduce\n");

	int i;
	vec128 ret;		

	ret = a[0];
	for (i = 1; i < GFBITS; i++)
		ret = vec128_or(ret, a[i]);

	return ret;
}

static inline void vec128_mul(vec128 *h, vec128 *f, const vec128 *g) // 1218
{
	//fprintf(stderr, "Error vec128_mul\n");
	//uint32_t t1 = ccnt_read();
  //transpose128_time_count += t1-t0;
  //transpose128_count += 1;
	//uint32_t t0 = ccnt_read();

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
		buf[i-GFBITS+3] ^= buf[i]; 
		buf[i-GFBITS+0] ^= buf[i]; 
	}

	for (i = 0; i < GFBITS; i++)
		h[i] = buf[i];
	//uint32_t t1 = ccnt_read();
  //transpose128_time_count += t1-t0;
  //transpose128_count += 1;

}

void vec128_sq(vec128 *, vec128 *);
void vec128_inv(vec128 *, vec128 *);

#endif
