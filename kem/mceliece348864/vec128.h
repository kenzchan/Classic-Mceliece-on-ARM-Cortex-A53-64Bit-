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
#include <string.h>

//#include <smmintrin.h>

typedef uint64x2_t vec128;


extern uint64_t transpose128_time_count;
extern uint64_t transpose128_count;


static inline uint64_t ccnt_read()
{
  uint64_t t = 0;
  asm volatile("mrs %0, PMCCNTR_EL0":"=r"(t));
  return t;
}


static void print128_num(uint64x2_t var)
{
    uint16_t val[8];
    memcpy(val, &var, sizeof(val));
    fprintf(stderr, "Numerical: %i %i %i %i %i %i %i %i \n", 
           val[0], val[1], 
           val[2], val[3],
           val[4], val[5], 
           val[6], val[7]);
}



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
static inline void vec128_mul_test(vec128 *h, vec128 *f, const vec128 *g)
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

/*
static void vec128_mul_asm(uint64x2_t *h, uint64x2_t *f, const uint64x2_t *g)
{
	asm volatile (
		"ldr q0, [%[x1]] \n"
		"ldr q1, [%[x2]] \n"
		"and v0.16b, v0.16b, v1.16b \n"
		"str q0, [%[x0]] \n"
    : [x0] "+r" (h)
    : [x1] "r" (f), [x2] "r" (g)
    : "memory"
  );
	fprintf(stderr, "------\n" );
}
*/

static inline void vec128_mul_correct2(uint64x2_t *h, uint64x2_t *f, const uint64x2_t *g)
{
	uint64x2_t buf[2*GFBITS - 1];

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


static inline void vec128_mul_asm(vec128 *h, vec128 *f, const vec128 *g)
{
  __asm__ __volatile (
    "sub sp, sp, #(16*18) \n"
		"ldp q3, q7, [%[x2]] \n"
		"stp d8, d9, [sp, #-256]! \n"
		"ldp q5, q6, [%[x1], #80] \n"
		"stp d12, d13, [sp, #32] \n"
		"ldp q22, q24, [%[x2], #64] \n"
		"stp d10, d11, [sp, #16] \n"
		"ldr q28, [%[x2], #96] \n"
		"stp d14, d15, [sp, #48] \n"
		"ldp q27, q26, [%[x1], #160] \n"
		"mov v13.16b, v3.16b \n"
		"ldp q8, q18, [%[x1]] \n"
		"and v1.16b, v3.16b, v5.16b \n"
		"and v2.16b, v7.16b, v6.16b \n"
		"ldp q16, q25, [%[x1], #128] \n"
		"and v0.16b, v24.16b, v6.16b \n"
		"ldr q29, [%[x2], #176] \n"
		"str q1, [sp, #240] \n"
		"and v1.16b, v5.16b, v28.16b \n"
		"str q2, [sp, #112] \n"
		"and v10.16b, v13.16b, v27.16b \n"
		"ldr q21, [%[x2], #32] \n"
		"stp q7, q13, [sp, #80] \n"
		"eor v0.16b, v0.16b, v1.16b \n"
		"and v1.16b, v13.16b, v26.16b \n"
		"ldr q23, [%[x2], #144] \n"
		"and v4.16b, v8.16b, v29.16b \n"
		"and v2.16b, v7.16b, v25.16b \n"
		"and v11.16b, v3.16b, v25.16b \n"
		"and v30.16b, v18.16b, v29.16b \n"
		"and v3.16b, v7.16b, v27.16b \n"
		"and v12.16b, v21.16b, v16.16b \n"
		"eor v0.16b, v0.16b, v1.16b \n"
		"ldr q1, [%[x2], #48] \n"
		"and v31.16b, v21.16b, v26.16b \n"
		"eor v10.16b, v10.16b, v2.16b \n"
		"and v2.16b, v7.16b, v26.16b \n"
		"ldr q7, [%[x1], #112] \n"
		"and v9.16b, v21.16b, v25.16b \n"
		"eor v0.16b, v0.16b, v4.16b \n"
		"ldr q4, [%[x1], #64] \n"
		"eor v30.16b, v30.16b, v2.16b \n"
		"eor v10.16b, v10.16b, v12.16b \n"
		"and v12.16b, v1.16b, v27.16b \n"
		"and v15.16b, v1.16b, v16.16b \n"
		"eor v0.16b, v0.16b, v3.16b \n"
		"ldr q3, [%[x2], #128] \n"
		"str q8, [sp, #64] \n"
		"and v8.16b, v8.16b, v23.16b \n"
		"ldr q2, [%[x2], #160] \n"
		"eor v31.16b, v31.16b, v12.16b \n"
		"and v12.16b, v1.16b, v7.16b \n"
		"and v13.16b, v4.16b, v22.16b \n"
		"ldp q19, q20, [%[x1], #32] \n"
		"eor v8.16b, v11.16b, v8.16b \n"
		"and v11.16b, v21.16b, v27.16b \n"
		"eor v10.16b, v10.16b, v12.16b \n"
		"ldr q17, [%[x2], #112] \n"
		"and v14.16b, v18.16b, v2.16b \n"
		"eor v30.16b, v30.16b, v11.16b \n"
		"ldr q11, [sp, #64] \n"
		"eor v0.16b, v0.16b, v14.16b \n"
		"ldp q14, q12, [sp, #64] \n"
		"and v11.16b, v11.16b, v3.16b \n"
		"eor v0.16b, v0.16b, v9.16b \n"
		"and v9.16b, v19.16b, v23.16b \n"
		"eor v11.16b, v13.16b, v11.16b \n"
		"eor v0.16b, v0.16b, v9.16b \n"
		"and v9.16b, v20.16b, v3.16b \n"
		"and v13.16b, v12.16b, v16.16b \n"
		"and v12.16b, v22.16b, v25.16b \n"
		"and v14.16b, v14.16b, v17.16b \n"
		"eor v0.16b, v0.16b, v9.16b \n"
		"and v9.16b, v22.16b, v6.16b \n"
		"eor v8.16b, v8.16b, v13.16b \n"
		"ldr q13, [sp, #96] \n"
		"eor v31.16b, v31.16b, v12.16b \n"
		"and v12.16b, v1.16b, v25.16b \n"
		"eor v10.16b, v10.16b, v9.16b \n"
		"eor v0.16b, v0.16b, v15.16b \n"
		"and v15.16b, v24.16b, v5.16b \n"
		"eor v30.16b, v30.16b, v12.16b \n"
		"and v9.16b, v13.16b, v7.16b \n"
		"and v12.16b, v13.16b, v16.16b \n"
		"and v13.16b, v22.16b, v7.16b \n"
		"eor v9.16b, v14.16b, v9.16b \n"
		"eor v14.16b, v10.16b, v15.16b \n"
		"ldr q15, [sp, #112] \n"
		"and v10.16b, v24.16b, v16.16b \n"
		"eor v11.16b, v11.16b, v12.16b \n"
		"and v12.16b, v18.16b, v3.16b \n"
		"eor v0.16b, v0.16b, v13.16b \n"
		"and v13.16b, v4.16b, v17.16b \n"
		"str q14, [sp, #144] \n"
		"and v14.16b, v1.16b, v26.16b \n"
		"eor v31.16b, v31.16b, v10.16b \n"
		"and v10.16b, v23.16b, v26.16b \n"
		"eor v9.16b, v9.16b, v15.16b \n"
		"eor v8.16b, v8.16b, v12.16b \n"
		"and v12.16b, v22.16b, v27.16b \n"
		"eor v0.16b, v0.16b, v13.16b \n"
		"and v13.16b, v25.16b, v29.16b \n"
		"and v15.16b, v22.16b, v16.16b \n"
		"str q9, [sp, #192] \n"
		"and v9.16b, v27.16b, v2.16b \n"
		"eor v14.16b, v14.16b, v12.16b \n"
		"and v12.16b, v22.16b, v26.16b \n"
		"eor v30.16b, v30.16b, v15.16b \n"
		"and v15.16b, v1.16b, v20.16b \n"
		"eor v9.16b, v10.16b, v9.16b \n"
		"and v10.16b, v24.16b, v27.16b \n"
		"eor v12.16b, v12.16b, v10.16b \n"
		"ldr q10, [sp, #80] \n"
		"eor v9.16b, v9.16b, v13.16b \n"
		"eor v0.16b, v0.16b, v9.16b \n"
		"and v13.16b, v10.16b, v7.16b \n"
		"and v10.16b, v21.16b, v7.16b \n"
		"str q0, [%[x0], #176] \n"
		"and v0.16b, v18.16b, v17.16b \n"
		"eor v11.16b, v11.16b, v13.16b \n"
		"ldr q13, [sp, #96] \n"
		"eor v8.16b, v8.16b, v10.16b \n"
		"eor v11.16b, v11.16b, v0.16b \n"
		"and v0.16b, v19.16b, v28.16b \n"
		"and v10.16b, v13.16b, v6.16b \n"
		"and v13.16b, v24.16b, v25.16b \n"
		"eor v15.16b, v15.16b, v10.16b \n"
		"eor v10.16b, v14.16b, v13.16b \n"
		"and v14.16b, v24.16b, v7.16b \n"
		"and v13.16b, v21.16b, v6.16b \n"
		"eor v13.16b, v11.16b, v13.16b \n"
		"eor v11.16b, v30.16b, v14.16b \n"
		"and v30.16b, v24.16b, v26.16b \n"
		"and v14.16b, v28.16b, v27.16b \n"
		"stp q10, q11, [sp, #112] \n"
		"and v10.16b, v28.16b, v7.16b \n"
		"eor v0.16b, v13.16b, v0.16b \n"
		"eor v13.16b, v30.16b, v14.16b \n"
		"ldr q30, [sp, #144] \n"
		"and v14.16b, v4.16b, v28.16b \n"
		"and v11.16b, v1.16b, v5.16b \n"
		"eor v31.16b, v31.16b, v10.16b \n"
		"and v10.16b, v28.16b, v25.16b \n"
		"str q13, [sp, #176] \n"
		"and v13.16b, v28.16b, v26.16b \n"
		"eor v0.16b, v0.16b, v11.16b \n"
		"and v11.16b, v20.16b, v24.16b \n"
		"eor v14.16b, v30.16b, v14.16b \n"
		"ldr q30, [sp, #192] \n"
		"eor v12.16b, v12.16b, v10.16b \n"
		"and v10.16b, v19.16b, v17.16b \n"
		"eor v0.16b, v0.16b, v11.16b \n"
		"str q12, [sp, #160] \n"
		"and v12.16b, v18.16b, v28.16b \n"
		"eor v8.16b, v8.16b, v10.16b \n"
		"ldr q10, [sp, #240] \n"
		"eor v9.16b, v0.16b, v9.16b \n"
		"and v0.16b, v5.16b, v3.16b \n"
		"eor v12.16b, v30.16b, v12.16b \n"
		"ldr q30, [sp, #64] \n"
		"str q9, [sp, #208] \n"
		"and v9.16b, v20.16b, v17.16b \n"
		"str q12, [sp, #224] \n"
		"and v12.16b, v6.16b, v17.16b \n"
		"and v11.16b, v30.16b, v28.16b \n"
		"eor v14.16b, v14.16b, v9.16b \n"
		"and v9.16b, v19.16b, v3.16b \n"
		"eor v31.16b, v31.16b, v12.16b \n"
		"eor v15.16b, v15.16b, v11.16b \n"
		"and v11.16b, v17.16b, v27.16b \n"
		"eor v14.16b, v14.16b, v9.16b \n"
		"and v9.16b, v18.16b, v23.16b \n"
		"eor v31.16b, v31.16b, v0.16b \n"
		"and v0.16b, v4.16b, v23.16b \n"
		"eor v12.16b, v13.16b, v11.16b \n"
		"and v11.16b, v30.16b, v24.16b \n"
		"and v13.16b, v28.16b, v16.16b \n"
		"eor v14.16b, v14.16b, v9.16b \n"
		"eor v31.16b, v31.16b, v0.16b \n"
		"and v0.16b, v20.16b, v2.16b \n"
		"and v9.16b, v30.16b, v2.16b \n"
		"and v30.16b, v6.16b, v28.16b \n"
		"and v28.16b, v20.16b, v28.16b \n"
		"eor v10.16b, v11.16b, v10.16b \n"
		"ldr q11, [sp, #112] \n"
		"eor v31.16b, v31.16b, v0.16b \n"
		"and v0.16b, v19.16b, v29.16b \n"
		"eor v9.16b, v14.16b, v9.16b \n"
		"ldr q14, [sp, #128] \n"
		"str q10, [sp, #240] \n"
		"eor v31.16b, v31.16b, v0.16b \n"
		"eor v0.16b, v8.16b, v28.16b \n"
		"eor v10.16b, v11.16b, v13.16b \n"
		"and v13.16b, v3.16b, v26.16b \n"
		"and v11.16b, v26.16b, v29.16b \n"
		"and v28.16b, v3.16b, v27.16b \n"
		"eor v14.16b, v14.16b, v30.16b \n"
		"and v30.16b, v17.16b, v25.16b \n"
		"str q0, [sp, #192] \n"
		"and v0.16b, v23.16b, v27.16b \n"
		"and v27.16b, v27.16b, v29.16b \n"
		"eor v8.16b, v9.16b, v11.16b \n"
		"eor v9.16b, v31.16b, v11.16b \n"
		"ldr q11, [sp, #176] \n"
		"eor v0.16b, v13.16b, v0.16b \n"
		"and v13.16b, v25.16b, v2.16b \n"
		"stp q9, q10, [sp, #128] \n"
		"eor v0.16b, v0.16b, v13.16b \n"
		"and v13.16b, v16.16b, v29.16b \n"
		"and v9.16b, v17.16b, v26.16b \n"
		"eor v30.16b, v11.16b, v30.16b \n"
		"ldr q11, [sp, #160] \n"
		"and v10.16b, v17.16b, v16.16b \n"
		"and v26.16b, v2.16b, v26.16b \n"
		"ldr q31, [sp, #224] \n"
		"eor v0.16b, v0.16b, v13.16b \n"
		"eor v9.16b, v9.16b, v28.16b \n"
		"ldr q28, [sp, #80] \n"
		"eor v26.16b, v27.16b, v26.16b \n"
		"and v27.16b, v21.16b, v19.16b \n"
		"eor v10.16b, v11.16b, v10.16b \n"
		"and v11.16b, v21.16b, v5.16b \n"
		"eor v8.16b, v8.16b, v0.16b \n"
		"str q26, [sp, #112] \n"
		"and v26.16b, v18.16b, v24.16b \n"
		"and v13.16b, v28.16b, v5.16b \n"
		"and v28.16b, v28.16b, v4.16b \n"
		"str q8, [%[x0], #160] \n"
		"eor v11.16b, v31.16b, v11.16b \n"
		"and v31.16b, v3.16b, v25.16b \n"
		"ldr q8, [sp, #240] \n"
		"and v25.16b, v25.16b, v23.16b \n"
		"eor v15.16b, v15.16b, v13.16b \n"
		"and v13.16b, v17.16b, v7.16b \n"
		"eor v31.16b, v12.16b, v31.16b \n"
		"and v12.16b, v19.16b, v24.16b \n"
		"eor v25.16b, v9.16b, v25.16b \n"
		"and v9.16b, v1.16b, v4.16b \n"
		"and v17.16b, v5.16b, v17.16b \n"
		"and v24.16b, v4.16b, v24.16b \n"
		"eor v28.16b, v8.16b, v28.16b \n"
		"ldr q8, [sp, #144] \n"
		"eor v15.16b, v15.16b, v26.16b \n"
		"eor v11.16b, v11.16b, v12.16b \n"
		"and v12.16b, v7.16b, v3.16b \n"
		"and v26.16b, v6.16b, v3.16b \n"
		"eor v17.16b, v14.16b, v17.16b \n"
		"ldr q14, [sp, #192] \n"
		"eor v11.16b, v11.16b, v9.16b \n"
		"and v9.16b, v20.16b, v22.16b \n"
		"eor v13.16b, v8.16b, v13.16b \n"
		"and v8.16b, v3.16b, v16.16b \n"
		"eor v12.16b, v10.16b, v12.16b \n"
		"and v3.16b, v4.16b, v3.16b \n"
		"eor v9.16b, v11.16b, v9.16b \n"
		"and v11.16b, v16.16b, v23.16b \n"
		"eor v30.16b, v30.16b, v8.16b \n"
		"ldr q8, [sp, #96] \n"
		"and v16.16b, v16.16b, v2.16b \n"
		"eor v3.16b, v17.16b, v3.16b \n"
		"and v17.16b, v18.16b, v21.16b \n"
		"eor v13.16b, v13.16b, v26.16b \n"
		"and v26.16b, v21.16b, v4.16b \n"
		"eor v31.16b, v31.16b, v11.16b \n"
		"ldr q11, [sp, #64] \n"
		"eor v9.16b, v9.16b, v0.16b \n"
		"and v0.16b, v7.16b, v23.16b \n"
		"eor v16.16b, v25.16b, v16.16b \n"
		"and v25.16b, v22.16b, v5.16b \n"
		"and v8.16b, v8.16b, v4.16b \n"
		"eor v15.16b, v15.16b, v26.16b \n"
		"and v26.16b, v4.16b, v2.16b \n"
		"and v4.16b, v4.16b, v29.16b \n"
		"eor v30.16b, v30.16b, v0.16b \n"
		"and v0.16b, v6.16b, v2.16b \n"
		"and v10.16b, v11.16b, v22.16b \n"
		"and v11.16b, v6.16b, v23.16b \n"
		"eor v27.16b, v27.16b, v8.16b \n"
		"and v8.16b, v1.16b, v6.16b \n"
		"and v6.16b, v6.16b, v29.16b \n"
		"eor v30.16b, v30.16b, v0.16b \n"
		"ldr q0, [sp, #96] \n"
		"eor v12.16b, v12.16b, v11.16b \n"
		"and v11.16b, v5.16b, v2.16b \n"
		"eor v8.16b, v14.16b, v8.16b \n"
		"and v14.16b, v18.16b, v22.16b \n"
		"eor v27.16b, v27.16b, v10.16b \n"
		"and v10.16b, v21.16b, v20.16b \n"
		"and v22.16b, v19.16b, v22.16b \n"
		"eor v12.16b, v12.16b, v11.16b \n"
		"eor v28.16b, v28.16b, v14.16b \n"
		"ldr q14, [sp, #80] \n"
		"eor v8.16b, v8.16b, v24.16b \n"
		"and v24.16b, v5.16b, v23.16b \n"
		"and v23.16b, v20.16b, v23.16b \n"
		"and v5.16b, v5.16b, v29.16b \n"
		"eor v15.16b, v15.16b, v22.16b \n"
		"eor v12.16b, v12.16b, v4.16b \n"
		"ldr q4, [sp, #112] \n"
		"eor v28.16b, v28.16b, v10.16b \n"
		"eor v8.16b, v8.16b, v25.16b \n"
		"ldr q10, [sp, #80] \n"
		"and v25.16b, v7.16b, v29.16b \n"
		"and v7.16b, v7.16b, v2.16b \n"
		"and v14.16b, v14.16b, v19.16b \n"
		"and v2.16b, v19.16b, v2.16b \n"
		"eor v3.16b, v3.16b, v23.16b \n"
		"eor v13.16b, v13.16b, v24.16b \n"
		"eor v30.16b, v30.16b, v5.16b \n"
		"eor v31.16b, v31.16b, v7.16b \n"
		"eor v16.16b, v16.16b, v25.16b \n"
		"eor v14.16b, v14.16b, v17.16b \n"
		"ldr q17, [sp, #64] \n"
		"and v25.16b, v19.16b, v1.16b \n"
		"and v7.16b, v10.16b, v20.16b \n"
		"eor v3.16b, v3.16b, v2.16b \n"
		"ldr q2, [sp, #64] \n"
		"and v22.16b, v10.16b, v18.16b \n"
		"and v19.16b, v0.16b, v19.16b \n"
		"eor v6.16b, v31.16b, v6.16b \n"
		"eor v13.16b, v13.16b, v26.16b \n"
		"eor v28.16b, v28.16b, v25.16b \n"
		"eor v15.16b, v15.16b, v16.16b \n"
		"eor v27.16b, v27.16b, v7.16b \n"
		"mov v7.16b, v10.16b \n"
		"ldp d10, d11, [sp, #16] \n"
		"and v24.16b, v17.16b, v1.16b \n"
		"and v21.16b, v17.16b, v21.16b \n"
		"and v1.16b, v18.16b, v1.16b \n"
		"and v17.16b, v0.16b, v20.16b \n"
		"and v18.16b, v0.16b, v18.16b \n"
		"and v20.16b, v20.16b, v29.16b \n"
		"and v31.16b, v2.16b, v7.16b \n"
		"eor v8.16b, v8.16b, v4.16b \n"
		"eor v14.16b, v14.16b, v24.16b \n"
		"eor v21.16b, v22.16b, v21.16b \n"
		"eor v1.16b, v27.16b, v1.16b \n"
		"and v27.16b, v2.16b, v0.16b \n"
		"ldr q0, [sp, #128] \n"
		"eor v13.16b, v13.16b, v20.16b \n"
		"eor v3.16b, v3.16b, v4.16b \n"
		"ldr q2, [sp, #208] \n"
		"eor v28.16b, v28.16b, v6.16b \n"
		"eor v31.16b, v31.16b, v18.16b \n"
		"eor v14.16b, v14.16b, v17.16b \n"
		"eor v21.16b, v21.16b, v19.16b \n"
		"eor v1.16b, v1.16b, v30.16b \n"
		"eor v9.16b, v9.16b, v30.16b \n"
		"eor v16.16b, v8.16b, v16.16b \n"
		"eor v27.16b, v27.16b, v3.16b \n"
		"eor v28.16b, v28.16b, v13.16b \n"
		"eor v31.16b, v31.16b, v0.16b \n"
		"eor v14.16b, v14.16b, v12.16b \n"
		"eor v21.16b, v21.16b, v13.16b \n"
		"eor v12.16b, v15.16b, v12.16b \n"
		"eor v6.16b, v2.16b, v6.16b \n"
		"eor v1.16b, v1.16b, v0.16b \n"
		"stp q27, q31, [%[x0]] \n"
		"eor v14.16b, v14.16b, v3.16b \n"
		"stp q1, q28, [%[x0], #64] \n"
		"stp q12, q9, [%[x0], #96] \n"
		"stp q21, q14, [%[x0], #32] \n"
		"stp q6, q16, [%[x0], #128] \n"
		"ldp d12, d13, [sp, #32] \n"
		"ldp d14, d15, [sp, #48] \n"
		"ldp d8, d9, [sp], #256 \n"
  	"add sp, sp, #(16*18) \n"
    : [x0] "+r" (h)
    : [x1] "r" (f), [x2] "r" (g)
    :	"q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "q16", "q17", "q18", "q19", "q20", "q21", "q22", "q23", "q24", "q25", "q26", "q27", "q28", "q29", "q30", "q31", "cc", "memory"
  );
	//fprintf(stderr, "-\n");
}

/*
static void vec128_mul_correct(uint64x2_t *h, uint64x2_t *f, const uint64x2_t *g)
{
	uint64x2_t buf[ 2*GFBITS-1 ];
	uint64x2_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9;

	buf[0] = vandq_u64(f[0], g[0]);

	tmp1 = vandq_u64(f[0], g[1]);

	tmp3 = vandq_u64(f[1], g[1]);
	tmp4 = vandq_u64(f[0], g[2]);
	tmp5 = vandq_u64(f[2], g[0]);

	tmp2 = vandq_u64(f[1], g[0]);

	buf[1] = veorq_u64(tmp1, tmp2);//1, 2

	tmp6 = vandq_u64(f[2], g[1]);
	tmp4 = veorq_u64(tmp4, tmp5);//5
	tmp7 = vandq_u64(f[1], g[2]);
	tmp8 = vandq_u64(f[0], g[3]);
	tmp5 = vandq_u64(f[3], g[0]);

	buf[2] = veorq_u64(tmp3, tmp4); //3, 4, 5

	tmp6 = veorq_u64(tmp6, tmp7);//7

	tmp1 = vandq_u64(f[2], g[2]);

	tmp8 = veorq_u64(tmp8, tmp5);//5

	tmp2 = vandq_u64(f[4], g[0]);
	tmp3 = vandq_u64(f[0], g[4]);
	tmp4 = vandq_u64(f[3], g[1]);
	tmp5 = vandq_u64(f[1], g[3]);
	tmp1 = veorq_u64(tmp1, tmp2);//2

	buf[3] = veorq_u64(tmp6, tmp8);//6,8
	tmp3 = veorq_u64(tmp3, tmp4);//4

	tmp2 = vandq_u64(f[4], g[1]);
	tmp6 = vandq_u64(f[0], g[5]);
	tmp7 = vandq_u64(f[5], g[0]);

	tmp1 = veorq_u64(tmp1, tmp3);//3

	tmp8 = vandq_u64(f[1], g[4]);
	tmp9 = vandq_u64(f[3], g[2]);
	tmp3 = vandq_u64(f[2], g[3]);

	buf[4] = veorq_u64(tmp1, tmp5 );//1,2,3,4,5

	tmp6 = veorq_u64(tmp6, tmp2);//2
	tmp8 = veorq_u64(tmp8, tmp7);//7
	tmp3 = veorq_u64(tmp3, tmp9);//9


	tmp1 = vandq_u64(f[3], g[3]);
	tmp2 = vandq_u64(f[6], g[0]);
	tmp6 = veorq_u64(tmp6, tmp8);//8

	tmp4 = vandq_u64(f[0], g[6]);
	tmp5 = vandq_u64(f[5], g[1]);
	tmp7 = vandq_u64(f[1], g[5]);
	tmp9 = vandq_u64(f[4], g[2]);

	buf[5] = veorq_u64(tmp6, tmp3);//6, 3
	tmp3 = vandq_u64(f[2], g[4]);

	tmp1 = veorq_u64(tmp1, tmp2);//2
	tmp4 = veorq_u64(tmp4, tmp5);//5
	tmp7 = veorq_u64(tmp7, tmp9);//9

	//12 3 45 79

	tmp6 = vandq_u64(f[0], g[7]);
	tmp1 = veorq_u64(tmp1, tmp3);//3
	tmp3 = vandq_u64(f[2], g[5]);
	tmp9 = vandq_u64(f[5], g[2]);

	tmp8 = vandq_u64(f[7], g[0]);
	tmp4 = veorq_u64(tmp4, tmp7);//7
	tmp7 = vandq_u64(f[4], g[3]);

	tmp2 = vandq_u64(f[6], g[1]);
	tmp5 = vandq_u64(f[1], g[6]);
	tmp6 = veorq_u64(tmp6, tmp8);//8
	tmp8 = vandq_u64(f[4], g[4]);


	buf[6] = veorq_u64(tmp4, tmp1);//4, 1
	tmp1 = vandq_u64(f[8], g[0]);

	tmp4 = vandq_u64(f[3], g[4]);

	tmp2 = veorq_u64(tmp2, tmp5);//5

	tmp3 = veorq_u64(tmp3, tmp9);//9
	tmp9 = vandq_u64(f[7], g[1]);
	tmp2 = veorq_u64(tmp2, tmp6);//6	
	//68 25 93 74

	tmp4 = veorq_u64(tmp4, tmp7);//7
	tmp7 = vandq_u64(f[1], g[7]);
	tmp6 = vandq_u64(f[6], g[2]);
	tmp5 = vandq_u64(f[0], g[8]);
	tmp3 = veorq_u64(tmp3, tmp4);//4
	tmp4 = vandq_u64(f[2], g[6]);
	buf[7] = veorq_u64(tmp3, tmp2);//4

	//vandq_u64(f[5], g[3])
	//vandq_u64(f[3], g[5]);


	//buf[7] = veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[0], g[7]), vandq_u64(f[7], g[0])),veorq_u64(vandq_u64(f[6], g[1]), vandq_u64(f[1], g[6]))),veorq_u64(veorq_u64(vandq_u64(f[5], g[2]), vandq_u64(f[2], g[5])),veorq_u64(vandq_u64(f[4], g[3]), vandq_u64(f[3], g[4]))));

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
*/


static inline void vec128_mul(vec128 *h, vec128 *f, const vec128 *g)
{
	uint64_t t0 = ccnt_read();

	vec128_mul_asm(h, f, g);

	uint64_t t1 = ccnt_read();
  transpose128_time_count += t1-t0;
 	transpose128_count += 1;
}



void vec128_sq(vec128 *, vec128 *);
void vec128_inv(vec128 *, vec128 *);

#endif
