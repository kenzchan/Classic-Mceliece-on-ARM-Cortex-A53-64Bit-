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


extern uint64_t transpose128_time_count;
extern uint64_t transpose128_count;

/*
static inline uint64_t ccnt_read()
{
  uint64_t t = 0;
  asm volatile("mrs %0, PMCCNTR_EL0":"=r"(t));
  return t;
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

/*
static inline void vec128_mul_test(vec128 *h, vec128 *f, const vec128 *g) // 1218
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
*/

/*
static inline void vec128_mul_test(vec128 *h, vec128 *f, const vec128 *g) // 1218
{

	int i, j;
	vec128 buf[ 2*GFBITS-1 ];

	for (j = 0; j < GFBITS; j++)
		buf[0+j] = vandq_u64(f[0], g[j]);

	for (i = 1; i < GFBITS; i++)
		buf[i+GFBITS-1] = vandq_u64(f[i], g[GFBITS-1]);

	for (i = 1; i < GFBITS; i++)
	for (j = 0; j < GFBITS-1; j++)
		buf[i+j] = veorq_u64(buf[i+j], vandq_u64(f[i], g[j]));
		
	for (i = 2*GFBITS-2; i >= GFBITS; i--)
	{
		buf[i-GFBITS+3] = veorq_u64(buf[i-GFBITS+3], buf[i]); 
		buf[i-GFBITS+0] = veorq_u64(buf[i-GFBITS+0], buf[i]); 
	}

	for (i = 0; i < GFBITS; i++)
		h[i] = buf[i];

}
*/

static void vec128_mul_test(uint64x2_t *h, uint64x2_t *f, const uint64x2_t *g)
{
	uint64x2_t buf[ 2*GFBITS-1 ];

	buf[0] = vandq_u64(f[0], g[0]);

	buf[1] = veorq_u64(vandq_u64(f[0], g[1]), vandq_u64(f[1], g[0]));

	buf[2] = veorq_u64(vandq_u64(f[1], g[1]), veorq_u64(vandq_u64(f[0], g[2]), vandq_u64(f[2], g[0])));

	buf[3] = veorq_u64(veorq_u64(vandq_u64(f[2], g[1]),vandq_u64(f[1], g[2])), veorq_u64(vandq_u64(f[0], g[3]), vandq_u64(f[3], g[0])));

	buf[4] = veorq_u64(vandq_u64(f[2], g[2]), veorq_u64(veorq_u64(vandq_u64(f[4], g[0]), vandq_u64(f[0], g[4])), veorq_u64(vandq_u64(f[3], g[1]), vandq_u64(f[1], g[3]))));

	buf[5] = veorq_u64(veorq_u64(vandq_u64(f[0], g[5]), vandq_u64(f[5], g[0])), veorq_u64(veorq_u64(vandq_u64(f[4], g[1]), vandq_u64(f[1], g[4])), veorq_u64(vandq_u64(f[3], g[2]), vandq_u64(f[2], g[3]))));
	
	buf[6] = veorq_u64(veorq_u64(vandq_u64(f[3], g[3]), veorq_u64(vandq_u64(f[6], g[0]), vandq_u64(f[0], g[6]))),veorq_u64(veorq_u64(vandq_u64(f[5], g[1]), vandq_u64(f[1], g[5])), veorq_u64(vandq_u64(f[4], g[2]), vandq_u64(f[2], g[4]))));
	
	buf[7] = veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[0], g[7]), vandq_u64(f[7], g[0])),veorq_u64(vandq_u64(f[6], g[1]), vandq_u64(f[1], g[6]))),veorq_u64(veorq_u64(vandq_u64(f[5], g[2]), vandq_u64(f[2], g[5])),veorq_u64(vandq_u64(f[4], g[3]), vandq_u64(f[3], g[4]))));

	buf[8] = veorq_u64(vandq_u64(f[4], g[4]), veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[0], g[8]), vandq_u64(f[8], g[0])),veorq_u64(vandq_u64(f[7], g[1]), vandq_u64(f[1], g[7]))),veorq_u64(veorq_u64(vandq_u64(f[6], g[2]), vandq_u64(f[2], g[6])),veorq_u64(vandq_u64(f[5], g[3]), vandq_u64(f[3], g[5])))));

	buf[9] = veorq_u64(veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[9], g[0]), vandq_u64(f[0], g[9])),veorq_u64(vandq_u64(f[8], g[1]), vandq_u64(f[1], g[8]))), veorq_u64(veorq_u64(vandq_u64(f[7], g[2]), vandq_u64(f[2], g[7])), veorq_u64(vandq_u64(f[3], g[6]), vandq_u64(f[6], g[3])))), veorq_u64(vandq_u64(f[4], g[5]), vandq_u64(f[5], g[4])));

	buf[10] = veorq_u64(vandq_u64(f[10], g[0]), veorq_u64(veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[9], g[1]), vandq_u64(f[8], g[2])),veorq_u64(vandq_u64(f[7], g[3]), vandq_u64(f[6], g[4]))), veorq_u64(veorq_u64(vandq_u64(f[5], g[5]), vandq_u64(f[4], g[6])), veorq_u64(vandq_u64(f[3], g[7]), vandq_u64(f[2], g[8])))), veorq_u64(vandq_u64(f[1], g[9]), vandq_u64(f[0], g[10]))));

	buf[11] = veorq_u64(veorq_u64(vandq_u64(f[6], g[5]), vandq_u64(f[5], g[6])), veorq_u64(veorq_u64(vandq_u64(f[11], g[0]), vandq_u64(f[0], g[11])), veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[10], g[1]), vandq_u64(f[1], g[10])), veorq_u64(vandq_u64(f[9], g[2]), vandq_u64(f[2], g[9]))), veorq_u64(veorq_u64(vandq_u64(f[3], g[8]), vandq_u64(f[8], g[3])), veorq_u64(vandq_u64(f[7], g[4]), vandq_u64(f[4], g[7]))))));

	buf[12] = veorq_u64(vandq_u64(f[1], g[11]), veorq_u64(veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[11], g[1]), vandq_u64(f[10], g[2])),veorq_u64(vandq_u64(f[9], g[3]), vandq_u64(f[8], g[4]))), veorq_u64(veorq_u64(vandq_u64(f[7], g[5]), vandq_u64(f[6], g[6])), veorq_u64(vandq_u64(f[5], g[7]), vandq_u64(f[4], g[8])))), veorq_u64(vandq_u64(f[3], g[9]), vandq_u64(f[2], g[10]))));

	buf[13] = veorq_u64(veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[11], g[2]), vandq_u64(f[10], g[3])),veorq_u64(vandq_u64(f[9], g[4]), vandq_u64(f[8], g[5]))), veorq_u64(veorq_u64(vandq_u64(f[7], g[6]), vandq_u64(f[6], g[7])), veorq_u64(vandq_u64(f[5], g[8]), vandq_u64(f[4], g[9])))), veorq_u64(vandq_u64(f[3], g[10]), vandq_u64(f[2], g[11])));

	buf[14] = veorq_u64(vandq_u64(f[11], g[3]), veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[10], g[4]), vandq_u64(f[9], g[5])),veorq_u64(vandq_u64(f[8], g[6]), vandq_u64(f[7], g[7]))),veorq_u64(veorq_u64(vandq_u64(f[6], g[8]), vandq_u64(f[5], g[9])),veorq_u64(vandq_u64(f[4], g[10]), vandq_u64(f[3], g[11])))));

	buf[15] = veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[11], g[4]), vandq_u64(f[10], g[5])),veorq_u64(vandq_u64(f[9], g[6]), vandq_u64(f[8], g[7]))),veorq_u64(veorq_u64(vandq_u64(f[7], g[8]), vandq_u64(f[6], g[9])),veorq_u64(vandq_u64(f[5], g[10]), vandq_u64(f[4], g[11]))));

	buf[16] = veorq_u64(veorq_u64(vandq_u64(f[11], g[5]), veorq_u64(vandq_u64(f[10], g[6]), vandq_u64(f[9], g[7]))),veorq_u64(veorq_u64(vandq_u64(f[8], g[8]), vandq_u64(f[7], g[9])), veorq_u64(vandq_u64(f[6], g[10]), vandq_u64(f[5], g[11]))));

	buf[17] = veorq_u64(veorq_u64(vandq_u64(f[11], g[6]), vandq_u64(f[10], g[7])), veorq_u64(veorq_u64(vandq_u64(f[9], g[8]), vandq_u64(f[8], g[9])), veorq_u64(vandq_u64(f[7], g[10]), vandq_u64(f[6], g[11]))));
	
	buf[18] = veorq_u64(vandq_u64(f[11], g[7]), veorq_u64(veorq_u64(vandq_u64(f[10], g[8]), vandq_u64(f[9], g[9])), veorq_u64(vandq_u64(f[8], g[10]), vandq_u64(f[7], g[11]))));

	buf[19] = veorq_u64(veorq_u64(vandq_u64(f[11], g[8]),vandq_u64(f[10], g[9])), veorq_u64(vandq_u64(f[9], g[10]), vandq_u64(f[8], g[11])));

	buf[20] = veorq_u64(vandq_u64(f[11], g[9]), veorq_u64(vandq_u64(f[10], g[10]), vandq_u64(f[9], g[11])));

	buf[21] = veorq_u64(vandq_u64(f[10], g[11]), vandq_u64(f[11], g[10]));

	buf[22] = vandq_u64(f[11], g[11]);

	buf[13] = veorq_u64(buf[13], buf[22]); 
	buf[10] = veorq_u64(buf[10], buf[22]);

	buf[12] = veorq_u64(buf[12], buf[21]); 
	buf[9] = veorq_u64(buf[9], buf[21]);

	h[11] = veorq_u64(buf[11], buf[20]); 
	buf[8] = veorq_u64(buf[8], buf[20]);

	h[10] = veorq_u64(buf[10], buf[19]); 
	buf[7] = veorq_u64(buf[7], buf[19]);

	h[9] = veorq_u64(buf[9], buf[18]); 
	buf[6] = veorq_u64(buf[6], buf[18]);

	h[8] = veorq_u64(buf[8], buf[17]); 
	buf[5] = veorq_u64(buf[5], buf[17]);

	h[7] = veorq_u64(buf[7], buf[16]); 
	buf[4] = veorq_u64(buf[4], buf[16]);

	h[6] = veorq_u64(buf[6], buf[15]); 
	buf[3] = veorq_u64(buf[3], buf[15]);

	h[5] = veorq_u64(buf[5], buf[14]); 
	h[2] = veorq_u64(buf[2], buf[14]);

	h[4] = veorq_u64(buf[4], buf[13]); 
	h[1] = veorq_u64(buf[1], buf[13]);

	h[3] = veorq_u64(buf[3], buf[12]); 
	h[0] = veorq_u64(buf[0], buf[12]);
}

static inline void vec128_mul(vec128 *h, vec128 *f, const vec128 *g)
{
	//uint64_t t0 = ccnt_read();

	vec128_mul_test(h, f, g);
	
	//uint64_t t1 = ccnt_read();
  //transpose128_time_count += t1-t0;
 	//transpose128_count += 1;
}

void vec128_sq(vec128 *, vec128 *);
void vec128_inv(vec128 *, vec128 *);

#endif
