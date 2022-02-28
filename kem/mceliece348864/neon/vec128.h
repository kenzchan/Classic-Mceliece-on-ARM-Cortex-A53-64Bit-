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

/*
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

static void vec128_mul(uint64x2_t *h, uint64x2_t *f, const uint64x2_t *g)
{
	asm volatile (
		// load g[10] g[11]
		"ldp q12, q13, [%[x2], #16*10] \n"
		"ldp q10, q11, [%[x1], #16*10] \n"//f[10],f[11]
		//compute buf with g[11]
		"and v31.16b, v13.16b, v11.16b \n"//buf[22] = f11g11
		"and v30.16b, v13.16b, v10.16b \n"//buf[21] = f10g11

		"ldp q8, q9, [%[x1], #16*8] \n"	//f[8], f[9]
		"and v29.16b, v13.16b, v9.16b \n"//buf[20] = f9g11
		"and v28.16b, v13.16b, v8.16b \n"//buf[19] = f8g11

		"ldp q6, q7, [%[x1], #16*6] \n"	//f[6], f[7]
		"and v27.16b, v13.16b, v7.16b \n"//buf[18] = f7g11
		"and v26.16b, v13.16b, v6.16b \n"//buf[17] = f6g11

		"ldp q4, q5, [%[x1], #16*4] \n"	//f[4], f[5]
		"and v25.16b, v13.16b, v5.16b \n"//buf[16] = f5g11
		"and v24.16b, v13.16b, v4.16b \n"//buf[15] = f4g11

		"ldp q2, q3, [%[x1], #16*2] \n"		//f[2], f[3]
		"and v23.16b, v13.16b, v3.16b \n"//buf[14] = f3g11
		"and v22.16b, v13.16b, v2.16b \n"//buf[13] = f2g11

		"ldp q0, q1, [%[x1]] \n"					//f[0], f[1]
		"and v21.16b, v13.16b, v1.16b \n"//buf[12] = f1g11
		"and v20.16b, v13.16b, v0.16b \n"//buf[11] = f0g11

		// compute buf with g[10]
		"and v14.16b, v12.16b, v11.16b \n"//tmp[21] = f11g10
		"and v15.16b, v12.16b, v10.16b \n"//tmp[20] = f10g10
		"and v16.16b, v12.16b, v9.16b \n"//tmp[19] = f9g10
		"and v17.16b, v12.16b, v8.16b \n"//tmp[18] = f8g10
		"and v18.16b, v12.16b, v7.16b \n"//tmp[17] = f7g10
		"and v19.16b, v12.16b, v6.16b \n"//tmp[16] = f6g10

		"eor v30.16b, v30.16b, v14.16b \n"//buf[21] = f10g11^f11g10
		"eor v29.16b, v29.16b, v15.16b \n"//buf[20] = f9g11^f10g10
		"eor v28.16b, v28.16b, v16.16b \n"//buf[19] = f8g11^f9g10
		"eor v27.16b, v27.16b, v17.16b \n"//buf[18] = f7g11^f8g10
		"eor v26.16b, v26.16b, v18.16b \n"//buf[17] = f6g11^f7g10
		"eor v25.16b, v25.16b, v19.16b \n"//buf[16] = f5g11^f6g10

		//"str q31, [%[x0], #16*22] \n" //store 10,11

		"and v14.16b, v12.16b, v5.16b \n"//tmp[15] = f5g10
		"and v15.16b, v12.16b, v4.16b \n"//tmp[14] = f4g10
		"and v16.16b, v12.16b, v3.16b \n"//tmp[13] = f3g10
		"eor v22.16b, v22.16b, v31.16b \n"//buf[13] = f2g11^(buf[22])
		"and v17.16b, v12.16b, v2.16b \n"//tmp[12] = f2g10
		"and v18.16b, v12.16b, v1.16b \n"//tmp[11] = f1g10
		"and v19.16b, v12.16b, v0.16b \n"//tmp[10] = f0g10

		"eor v24.16b, v24.16b, v14.16b \n"//buf[15] = f4g11^f5g10
		"eor v23.16b, v23.16b, v15.16b \n"//buf[14] = f3g11^f4g10
		"eor v22.16b, v22.16b, v16.16b \n"//buf[13] = f2g11^f3g10^(buf[22])
		"eor v21.16b, v21.16b, v17.16b \n"//buf[12] = f1g11^f2g10
		"eor v20.16b, v20.16b, v18.16b \n"//buf[11] = f0g11^f1g10
		"eor v31.16b, v31.16b, v19.16b \n"//buf[10] = f0g10^(buf[22])
		
		// load g[8] g[9]
		"ldp q12, q13, [%[x2], #16*8] \n"

		//compute buf with g9
		"and v14.16b, v13.16b, v11.16b \n"//tmp[20] = f11g9
		"and v15.16b, v13.16b, v10.16b \n"//tmp[19] = f10g9
		"and v16.16b, v13.16b, v9.16b \n"//tmp[18] = f9g9
		"and v17.16b, v13.16b, v8.16b \n"//tmp[17] = f8g9
		"and v18.16b, v13.16b, v7.16b \n"//tmp[16] = f7g9
		"and v19.16b, v13.16b, v6.16b \n"//tmp[15] = f6g9

		"eor v29.16b, v29.16b, v14.16b \n"//buf[20] = f9g11^f10g10^f11g9
		"eor v28.16b, v28.16b, v15.16b \n"//buf[19] = f8g11^f9g10^f10g9
		"eor v27.16b, v27.16b, v16.16b \n"//buf[18] = f7g11^f8g10^f9g9
		"eor v26.16b, v26.16b, v17.16b \n"//buf[17] = f6g11^f7g10^f8g9
		"eor v25.16b, v25.16b, v18.16b \n"//buf[16] = f5g11^f6g10^f7g9
		"eor v24.16b, v24.16b, v19.16b \n"//buf[15] = f4g11^f5g10^f6g9

		//"str q30, [%[x0], #16*21] \n" //store 10,11

		"and v14.16b, v13.16b, v5.16b \n"//tmp[14] = f5g9
		"and v15.16b, v13.16b, v4.16b \n"//tmp[13] = f4g9
		"and v16.16b, v13.16b, v3.16b \n"//tmp[12] = f3g9
		"eor v21.16b, v21.16b, v30.16b \n"//buf[12] = f1g11^f2g10^(buf[21])
		"and v17.16b, v13.16b, v2.16b \n"//tmp[11] = f2g9
		"and v18.16b, v13.16b, v1.16b \n"//tmp[10] = f1g9
		"and v19.16b, v13.16b, v0.16b \n"//tmp[9] = f0g9

		"eor v23.16b, v23.16b, v14.16b \n"//buf[14] = f3g11^f4g10
		"eor v22.16b, v22.16b, v15.16b \n"//buf[13] = f2g11^f3g10^(buf[22])
		"eor v21.16b, v21.16b, v16.16b \n"//buf[12] = f1g11^f2g10^(buf[21])
		"eor v20.16b, v20.16b, v17.16b \n"//buf[11] = f0g11^f1g10
		"eor v31.16b, v31.16b, v18.16b \n"//buf[10] = f0g10^(buf[22])
		"eor v30.16b, v30.16b, v19.16b \n"//buf[9] = f0g9^(buf[21])

		//compute buf with g[8]
		"and v14.16b, v12.16b, v11.16b \n"//tmp[19] = f11g8
		"and v15.16b, v12.16b, v10.16b \n"//tmp[18] = f10g8
		"and v16.16b, v12.16b, v9.16b \n"//tmp[17] = f9g8
		"and v17.16b, v12.16b, v8.16b \n"//tmp[16] = f8g8
		"and v18.16b, v12.16b, v7.16b \n"//tmp[15] = f7g8
		"and v19.16b, v12.16b, v6.16b \n"//tmp[14] = f6g8

		"eor v28.16b, v28.16b, v14.16b \n"//buf[19] = f8g11^f9g10^f10g9^f11g8
		"eor v27.16b, v27.16b, v15.16b \n"//buf[18] = f7g11^f8g10^f9g9^f10g8
		"eor v26.16b, v26.16b, v16.16b \n"//buf[17] = f6g11^f7g10^f8g9^f9g8
		"eor v25.16b, v25.16b, v17.16b \n"//buf[16] = f5g11^f6g10^f7g9^f8g8
		"eor v24.16b, v24.16b, v18.16b \n"//buf[15] = f4g11^f5g10^f6g9^f7g8
		"eor v23.16b, v23.16b, v19.16b \n"//buf[14] = f3g11^f4g10^f6g8

		//"str q29, [%[x0], #16*20] \n"

		"and v14.16b, v12.16b, v5.16b \n"//tmp[13] = f5g8
		"and v15.16b, v12.16b, v4.16b \n"//tmp[12] = f4g8
		"and v16.16b, v12.16b, v3.16b \n"//tmp[11] = f3g8
		"eor v20.16b, v20.16b, v29.16b \n"//buf[11] = f0g11^f1g10^(buf[20])
		"and v17.16b, v12.16b, v2.16b \n"//tmp[10] = f2g8
		"and v18.16b, v12.16b, v1.16b \n"//tmp[9] = f1g8
		"and v19.16b, v12.16b, v0.16b \n"//tmp[8] = f0g8

		"eor v22.16b, v22.16b, v14.16b \n"//buf[13] = f2g11^f3g10^f5g8^(buf[22])
		"eor v21.16b, v21.16b, v15.16b \n"//buf[12] = f1g11^f2g10^f4g8^(buf[21])
		"eor v20.16b, v20.16b, v16.16b \n"//buf[11] = f0g11^f1g10^f3g8^(buf[20])
		"eor v31.16b, v31.16b, v17.16b \n"//buf[10] = f0g10^f2g8^(buf[22])
		"eor v30.16b, v30.16b, v18.16b \n"//buf[9] = f0g9^f1g8^(buf[21])
		"eor v29.16b, v29.16b, v19.16b \n"//buf[8] = f0g8^(buf[20])

		
		//load g[6] g[7]
		"ldp q12, q13, [%[x2], #16*6] \n"

		//compute buf with g[7]
		"and v14.16b, v13.16b, v11.16b \n"//tmp[18] = f11g7
		"and v15.16b, v13.16b, v10.16b \n"//tmp[17] = f10g7
		"and v16.16b, v13.16b, v9.16b \n"//tmp[16] = f9g7
		"and v17.16b, v13.16b, v8.16b \n"//tmp[15] = f8g7
		"and v18.16b, v13.16b, v7.16b \n"//tmp[14] = f7g7
		"and v19.16b, v13.16b, v6.16b \n"//tmp[13] = f6g7

		"eor v27.16b, v27.16b, v14.16b \n"//buf[18] = f7g11^f8g10^f9g9^f10g8
		"eor v26.16b, v26.16b, v15.16b \n"//buf[17] = f6g11^f7g10^f8g9^f9g8
		"eor v25.16b, v25.16b, v16.16b \n"//buf[16] = f5g11^f6g10^f7g9^f8g8
		"eor v24.16b, v24.16b, v17.16b \n"//buf[15] = f4g11^f5g10^f6g9^f7g8
		"eor v23.16b, v23.16b, v18.16b \n"//buf[14] = f3g11^f4g10^f6g8
		"eor v22.16b, v22.16b, v19.16b \n"//buf[13] = f2g11^f3g10^f5g8^(buf[22])

		//"str q28, [%[x0], #16*19] \n"

		"and v14.16b, v13.16b, v5.16b \n"//tmp[12] = f5g7
		"and v15.16b, v13.16b, v4.16b \n"//tmp[11] = f4g7
		"and v16.16b, v13.16b, v3.16b \n"//tmp[10] = f3g7
		"eor v31.16b, v31.16b, v28.16b \n"//buf[10] = f0g10^f2g8^(buf[22]^buf[19])
		"and v17.16b, v13.16b, v2.16b \n"//tmp[9] = f2g7
		"and v18.16b, v13.16b, v1.16b \n"//tmp[8] = f1g7
		"and v19.16b, v13.16b, v0.16b \n"//tmp[7] = f0g7

		"eor v21.16b, v21.16b, v14.16b \n"//buf[12] = f1g11^f2g10^f4g8^f5g7^(buf[21])
		"eor v20.16b, v20.16b, v15.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^(buf[20])
		"eor v31.16b, v31.16b, v16.16b \n"//buf[10] = f0g10^f2g8^f3g7^(buf[22]^buf[19])
		"eor v30.16b, v30.16b, v17.16b \n"//buf[9] = f0g9^f1g8^f2g7^(buf[21])
		"eor v29.16b, v29.16b, v18.16b \n"//buf[8] = f0g8^f1g7^(buf[20])
		"eor v28.16b, v28.16b, v19.16b \n"//buf[7] = f0g7^(buf[19])

		
		//compute buf with g[6]
		"and v14.16b, v12.16b, v11.16b \n"//tmp[17] = f11g6
		"and v15.16b, v12.16b, v10.16b \n"//tmp[16] = f10g6
		"and v16.16b, v12.16b, v9.16b \n"//tmp[15] = f9g6
		"and v17.16b, v12.16b, v8.16b \n"//tmp[14] = f8g6
		"and v18.16b, v12.16b, v7.16b \n"//tmp[13] = f7g6
		"and v19.16b, v12.16b, v6.16b \n"//tmp[12] = f6g6

		"eor v26.16b, v26.16b, v14.16b \n"//buf[17] = f6g11^f7g10^f8g9^f9g8^f11g6
		"eor v25.16b, v25.16b, v15.16b \n"//buf[16] = f5g11^f6g10^f7g9^f8g8^f10g6
		"eor v24.16b, v24.16b, v16.16b \n"//buf[15] = f4g11^f5g10^f6g9^f7g8^f9g6
		"eor v23.16b, v23.16b, v17.16b \n"//buf[14] = f3g11^f4g10^f6g8^f8g6
		"eor v22.16b, v22.16b, v18.16b \n"//buf[13] = f2g11^f3g10^f5g8^f7g6^(buf[22])
		"eor v21.16b, v21.16b, v19.16b \n"//buf[12] = f1g11^f2g10^f4g8^f5g7^f6g6^(buf[21])

		//"str q27, [%[x0], #16*18] \n"

		"and v14.16b, v12.16b, v5.16b \n"//tmp[11] = f5g6
		"and v15.16b, v12.16b, v4.16b \n"//tmp[10] = f4g6
		"and v16.16b, v12.16b, v3.16b \n"//tmp[9] = f3g6
		"eor v30.16b, v30.16b, v27.16b \n"//buf[9] = f0g9^f1g8^f2g7^(buf[21]^buf[18])
		"and v17.16b, v12.16b, v2.16b \n"//tmp[8] = f2g6
		"and v18.16b, v12.16b, v1.16b \n"//tmp[7] = f1g6
		"and v19.16b, v12.16b, v0.16b \n"//tmp[6] = f0g6

		"eor v20.16b, v20.16b, v14.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^f5g6^(buf[20])
		"eor v31.16b, v31.16b, v15.16b \n"//buf[10] = f0g10^f2g8^f3g7^f4g6^(buf[22]^buf[19])
		"eor v30.16b, v30.16b, v16.16b \n"//buf[9] = f0g9^f1g8^f2g7^f3g6^(buf[21]^buf[18])
		"eor v29.16b, v29.16b, v17.16b \n"//buf[8] = f0g8^f1g7^f2g6^(buf[20])
		"eor v28.16b, v28.16b, v18.16b \n"//buf[7] = f0g7^f1g6^(buf[19])
		"eor v27.16b, v27.16b, v19.16b \n"//buf[6] = f0g6^(buf[18])

		
		//load g[4] g[5]
		"ldp q12, q13, [%[x2], #16*4] \n"

		//compute buf with g[5]
		"and v14.16b, v13.16b, v11.16b \n"//tmp[16] = f11g5
		"and v15.16b, v13.16b, v10.16b \n"//tmp[15] = f10g5
		"and v16.16b, v13.16b, v9.16b \n"//tmp[14] = f9g5
		"and v17.16b, v13.16b, v8.16b \n"//tmp[13] = f8g5
		"and v18.16b, v13.16b, v7.16b \n"//tmp[12] = f7g5
		"and v19.16b, v13.16b, v6.16b \n"//tmp[11] = f6g5

		"eor v25.16b, v25.16b, v14.16b \n"//buf[16] = f5g11^f6g10^f7g9^f8g8^f10g6^f11g5
		"eor v24.16b, v24.16b, v15.16b \n"//buf[15] = f4g11^f5g10^f6g9^f7g8^f9g6^f10g5
		"eor v23.16b, v23.16b, v16.16b \n"//buf[14] = f3g11^f4g10^f6g8^f8g6^f9g5
		"eor v22.16b, v22.16b, v17.16b \n"//buf[13] = f2g11^f3g10^f5g8^f7g6^f8g5^(buf[22])
		"eor v21.16b, v21.16b, v18.16b \n"//buf[12] = f1g11^f2g10^f4g8^f5g7^f6g6^f7g5^(buf[21])
		"eor v20.16b, v20.16b, v19.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^f5g6^f6g5^(buf[20])

		//"str q26, [%[x0], #16*17] \n"

		"and v14.16b, v13.16b, v5.16b \n"//tmp[10] = f5g5
		"and v15.16b, v13.16b, v4.16b \n"//tmp[9] = f4g5
		"and v16.16b, v13.16b, v3.16b \n"//tmp[8] = f3g5
		"eor v29.16b, v29.16b, v26.16b \n"//buf[8] = f0g8^f1g7^f2g6^(buf[20]^buf[17])
		"and v17.16b, v13.16b, v2.16b \n"//tmp[7] = f2g5
		"and v18.16b, v13.16b, v1.16b \n"//tmp[6] = f1g5
		"and v19.16b, v13.16b, v0.16b \n"//tmp[5] = f0g5

		"eor v31.16b, v31.16b, v14.16b \n"//buf[10] = f0g10^f2g8^f3g7^f4g6^f5g5^(buf[22]^buf[19])
		"eor v30.16b, v30.16b, v15.16b \n"//buf[9] = f0g9^f1g8^f2g7^f3g6^f4g5^(buf[21]^buf[18])
		"eor v29.16b, v29.16b, v16.16b \n"//buf[8] = f0g8^f1g7^f2g6^f3g5^(buf[20]^buf[17])
		"eor v28.16b, v28.16b, v17.16b \n"//buf[7] = f0g7^f1g6^f2g5^(buf[19])
		"eor v27.16b, v27.16b, v18.16b \n"//buf[6] = f0g6^f0g6^f1g5^(buf[18])
		"eor v26.16b, v26.16b, v19.16b \n"//buf[5] = f0g5^(buf[17])

		//compute buf with g[4]
		"and v14.16b, v12.16b, v11.16b \n"//tmp[15] = f11g4
		"and v15.16b, v12.16b, v10.16b \n"//tmp[14] = f10g4
		"and v16.16b, v12.16b, v9.16b \n"//tmp[13] = f9g4
		"and v17.16b, v12.16b, v8.16b \n"//tmp[12] = f8g4
		"and v18.16b, v12.16b, v7.16b \n"//tmp[11] = f7g4
		"and v19.16b, v12.16b, v6.16b \n"//tmp[10] = f6g4

		"eor v24.16b, v24.16b, v14.16b \n"//buf[15] = f4g11^f5g10^f6g9^f7g8^f9g6^f10g5^f11g4
		"eor v23.16b, v23.16b, v15.16b \n"//buf[14] = f3g11^f4g10^f6g8^f8g6^f9g5^f10g4
		"eor v22.16b, v22.16b, v16.16b \n"//buf[13] = f2g11^f3g10^f5g8^f7g6^f8g5^f9g4^(buf[22])
		"eor v21.16b, v21.16b, v17.16b \n"//buf[12] = f1g11^f2g10^f4g8^f5g7^f6g6^f7g5^f8g4^(buf[21])
		"eor v20.16b, v20.16b, v18.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^f5g6^f6g5^f7g4^(buf[20])
		"eor v31.16b, v31.16b, v19.16b \n"//buf[10] = f0g10^f2g8^f3g7^f4g6^f5g5^f6g4^(buf[22]^buf[19])

		//"str q25, [%[x0], #16*16] \n"

		"and v14.16b, v12.16b, v5.16b \n"//tmp[9] = f5g4
		"and v15.16b, v12.16b, v4.16b \n"//tmp[8] = f4g4
		"and v16.16b, v12.16b, v3.16b \n"//tmp[7] = f3g4
		"eor v28.16b, v28.16b, v25.16b \n"//buf[7] = f0g7^f1g6^f2g5^(buf[19]^buf[16])
		"and v17.16b, v12.16b, v2.16b \n"//tmp[6] = f2g4
		"and v18.16b, v12.16b, v1.16b \n"//tmp[5] = f1g4
		"and v19.16b, v12.16b, v0.16b \n"//tmp[4] = f0g4

		"eor v30.16b, v30.16b, v14.16b \n"//buf[9] = f0g9^f1g8^f2g7^f3g6^f4g5^f5g4^(buf[21]^buf[18])
		"eor v29.16b, v29.16b, v15.16b \n"//buf[8] = f0g8^f1g7^f2g6^f3g5^f4g4^(buf[20]^buf[17])
		"eor v28.16b, v28.16b, v16.16b \n"//buf[7] = f0g7^f1g6^f2g5^f3g4^(buf[19]^buf[16])
		"eor v27.16b, v27.16b, v17.16b \n"//buf[6] = f0g6^f0g6^f1g5^f2g4^(buf[18])
		"eor v26.16b, v26.16b, v18.16b \n"//buf[5] = f0g5^f1g4^(buf[17])
		"eor v25.16b, v25.16b, v19.16b \n"//buf[4] = f0g4^(buf[16])

		
		//load g[2], g[3]
		"ldp q12, q13, [%[x2], #16*2] \n"

		//compute buf with g[3]
		"and v14.16b, v13.16b, v11.16b \n"//tmp[14] = f11g3
		"and v15.16b, v13.16b, v10.16b \n"//tmp[13] = f10g3
		"and v16.16b, v13.16b, v9.16b \n"//tmp[12] = f9g3
		"and v17.16b, v13.16b, v8.16b \n"//tmp[11] = f8g3
		"and v18.16b, v13.16b, v7.16b \n"//tmp[10] = f7g3
		"and v19.16b, v13.16b, v6.16b \n"//tmp[9] = f6g3

		"eor v23.16b, v23.16b, v14.16b \n"//buf[14] = f3g11^f4g10^f6g8^f8g6^f9g5^f10g4^f11g3
		"eor v22.16b, v22.16b, v15.16b \n"//buf[13] = f2g11^f3g10^f5g8^f7g6^f8g5^f9g4^f10g3^(buf[22])
		"eor v21.16b, v21.16b, v16.16b \n"//buf[12] = f1g11^f2g10^f4g8^f5g7^f6g6^f7g5^f8g4^f9g3^(buf[21])
		"eor v20.16b, v20.16b, v17.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^f5g6^f6g5^f7g4^f8g3^(buf[20])
		"eor v31.16b, v31.16b, v18.16b \n"//buf[10] = f0g10^f2g8^f3g7^f4g6^f5g5^f6g4^f7g3^(buf[22]^buf[19])
		"eor v30.16b, v30.16b, v19.16b \n"//buf[9] = f0g9^f1g8^f2g7^f3g6^f4g5^f5g4^f6g3^(buf[21]^buf[18])

		//"str q24, [%[x0], #16*15] \n"

		"and v14.16b, v13.16b, v5.16b \n"//tmp[8] = f5g3
		"and v15.16b, v13.16b, v4.16b \n"//tmp[7] = f4g3
		"and v16.16b, v13.16b, v3.16b \n"//tmp[6] = f3g3
		"eor v27.16b, v27.16b, v24.16b \n"//buf[6] = f0g6^f0g6^f1g5^f2g4^(buf[18]^buf[15])
		"and v17.16b, v13.16b, v2.16b \n"//tmp[5] = f2g3
		"and v18.16b, v13.16b, v1.16b \n"//tmp[4] = f1g3
		"and v19.16b, v13.16b, v0.16b \n"//tmp[3] = f0g3

		"eor v29.16b, v29.16b, v14.16b \n"//buf[8] = f0g8^f1g7^f2g6^f3g5^f4g4^f5g3^(buf[20]^buf[17])
		"eor v28.16b, v28.16b, v15.16b \n"//buf[7] = f0g7^f1g6^f2g5^f3g4^f4g3^(buf[19]^buf[16])
		"eor v27.16b, v27.16b, v16.16b \n"//buf[6] = f0g6^f0g6^f1g5^f2g4^f3g3^(buf[18])
		"eor v26.16b, v26.16b, v17.16b \n"//buf[5] = f0g5^f1g4^f2g3^(buf[17])
		"eor v25.16b, v25.16b, v18.16b \n"//buf[4] = f0g4^f1g3^(buf[16])
		"eor v24.16b, v24.16b, v19.16b \n"//buf[3] = f0g3^(buf[15])

		
		//compute buf with g[2]
		"and v14.16b, v12.16b, v11.16b \n"//tmp[13] = f11g2
		"and v15.16b, v12.16b, v10.16b \n"//tmp[12] = f10g2
		"and v16.16b, v12.16b, v9.16b \n"//tmp[11] = f9g2
		"and v17.16b, v12.16b, v8.16b \n"//tmp[10] = f8g2
		"and v18.16b, v12.16b, v7.16b \n"//tmp[9] = f7g2
		"and v19.16b, v12.16b, v6.16b \n"//tmp[8] = f6g2

		"eor v22.16b, v22.16b, v14.16b \n"//buf[13] = f2g11^f3g10^f5g8^f7g6^f8g5^f9g4^f10g3^f11g2^(buf[22])
		"eor v21.16b, v21.16b, v15.16b \n"//buf[12] = f1g11^f2g10^f4g8^f5g7^f6g6^f7g5^f8g4^f9g3^f10g2^(buf[21])
		"eor v20.16b, v20.16b, v16.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^f5g6^f6g5^f7g4^f8g3^f9g2^(buf[20])
		"eor v31.16b, v31.16b, v17.16b \n"//buf[10] = f0g10^f2g8^f3g7^f4g6^f5g5^f6g4^f7g3^f8g2^(buf[22]^buf[19])
		"eor v30.16b, v30.16b, v18.16b \n"//buf[9] = f0g9^f1g8^f2g7^f3g6^f4g5^f5g4^f6g3^f7g2^(buf[21]^buf[18])
		"eor v29.16b, v29.16b, v19.16b \n"//buf[8] = f0g8^f1g7^f2g6^f3g5^f4g4^f5g3^f6g2^(buf[20]^buf[17])

		//"str q23, [%[x0], #16*14] \n"

		"and v14.16b, v12.16b, v5.16b \n"//tmp[7] = f5g2
		"and v15.16b, v12.16b, v4.16b \n"//tmp[6] = f4g2
		"and v16.16b, v12.16b, v3.16b \n"//tmp[5] = f3g2
		"eor v26.16b, v26.16b, v23.16b \n"//buf[5] = f0g5^f1g4^f2g3^(buf[17]^buf[14])
		"and v17.16b, v12.16b, v2.16b \n"//tmp[4] = f2g2
		"and v18.16b, v12.16b, v1.16b \n"//tmp[3] = f1g2
		"and v19.16b, v12.16b, v0.16b \n"//tmp[2] = f0g2

		"eor v28.16b, v28.16b, v14.16b \n"//buf[7] = f0g7^f1g6^f2g5^f3g4^f4g3^f5g2^(buf[19]^buf[16])
		"eor v27.16b, v27.16b, v15.16b \n"//buf[6] = f0g6^f0g6^f1g5^f2g4^f3g3^f4g2^(buf[18])
		"eor v26.16b, v26.16b, v16.16b \n"//buf[5] = f0g5^f1g4^f2g3^f3g2^(buf[17])
		"eor v25.16b, v25.16b, v17.16b \n"//buf[4] = f0g4^f1g3^f2g2^(buf[16])
		"eor v24.16b, v24.16b, v18.16b \n"//buf[3] = f0g3^f1g2^(buf[15])
		"eor v23.16b, v23.16b, v19.16b \n"//buf[2] = f0g2^(buf[14])

		
		//load g[0], g[1]
		"ldp q12, q13, [%[x2]] \n"

		//compute buf with g[1]
		"and v14.16b, v13.16b, v11.16b \n"//tmp[12] = f11g1
		"and v15.16b, v13.16b, v10.16b \n"//tmp[11] = f10g1
		"and v16.16b, v13.16b, v9.16b \n"//tmp[10] = f9g1
		"and v17.16b, v13.16b, v8.16b \n"//tmp[9] = f8g1
		"and v18.16b, v13.16b, v7.16b \n"//tmp[8] = f7g1
		"and v19.16b, v13.16b, v6.16b \n"//tmp[7] = f6g1

		"eor v21.16b, v21.16b, v14.16b \n"//buf[12] = f1g11^f2g10^f4g8^f5g7^f6g6^f7g5^f8g4^f9g3^f10g2^f11g1^(buf[21])
		"eor v20.16b, v20.16b, v15.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^f5g6^f6g5^f7g4^f8g3^f9g2^f10g1^(buf[20])
		"eor v31.16b, v31.16b, v16.16b \n"//buf[10] = f0g10^f2g8^f3g7^f4g6^f5g5^f6g4^f7g3^f8g2^f9g1^(buf[22]^buf[19])
		"eor v30.16b, v30.16b, v17.16b \n"//buf[9] = f0g9^f1g8^f2g7^f3g6^f4g5^f5g4^f6g3^f7g2^f8g1^(buf[21]^buf[18])
		"eor v29.16b, v29.16b, v18.16b \n"//buf[8] = f0g8^f1g7^f2g6^f3g5^f4g4^f5g3^f6g2^f7g1^(buf[20]^buf[17])
		"eor v28.16b, v28.16b, v19.16b \n"//buf[7] = f0g7^f1g6^f2g5^f3g4^f4g3^f5g2^f6g1^(buf[19]^buf[16])

		//"str q22, [%[x0], #16*13] \n"

		"and v14.16b, v13.16b, v5.16b \n"//tmp[6] = f5g1
		"and v15.16b, v13.16b, v4.16b \n"//tmp[5] = f4g1
		"and v16.16b, v13.16b, v3.16b \n"//tmp[4] = f3g1
		"eor v25.16b, v25.16b, v22.16b \n"//buf[4] = f0g4^f1g3^f2g2^(buf[16]^buf[13])
		"and v17.16b, v13.16b, v2.16b \n"//tmp[3] = f2g1
		"and v18.16b, v13.16b, v1.16b \n"//tmp[2] = f1g1
		"and v19.16b, v13.16b, v0.16b \n"//tmp[1] = f0g1

		"eor v27.16b, v27.16b, v14.16b \n"//buf[6] = f0g6^f0g6^f1g5^f2g4^f3g3^f4g2^f5g1^(buf[18])
		"eor v26.16b, v26.16b, v15.16b \n"//buf[5] = f0g5^f1g4^f2g3^f3g2^f4g1^(buf[17])
		"eor v25.16b, v25.16b, v16.16b \n"//buf[4] = f0g4^f1g3^f2g2^f3g1^(buf[16])
		"eor v24.16b, v24.16b, v17.16b \n"//buf[3] = f0g3^f1g2^f2g1^(buf[15])
		"eor v23.16b, v23.16b, v18.16b \n"//buf[2] = f0g2^f1g1^(buf[14])
		"eor v22.16b, v22.16b, v19.16b \n"//buf[1] = f0g1^(buf[13])

		//compute buf with g[0]
		"and v14.16b, v12.16b, v11.16b \n"//tmp[11] = f11g0
		"and v15.16b, v12.16b, v10.16b \n"//tmp[10] = f10g0
		"and v16.16b, v12.16b, v9.16b \n"//tmp[9] = f9g0
		"and v17.16b, v12.16b, v8.16b \n"//tmp[8] = f8g0
		"and v18.16b, v12.16b, v7.16b \n"//tmp[7] = f7g0
		"and v19.16b, v12.16b, v6.16b \n"//tmp[6] = f6g0

		"eor v20.16b, v20.16b, v14.16b \n"//buf[11] = f0g11^f1g10^f3g8^f4g7^f5g6^f6g5^f7g4^f8g3^f9g2^f10g1^f11g0^(buf[20])
		"eor v31.16b, v31.16b, v15.16b \n"//buf[10] = f0g10^f2g8^f3g7^f4g6^f5g5^f6g4^f7g3^f8g2^f9g1^f10g0^(buf[22]^buf[19])
		"stp q31, q20, [%[x0], #16*10] \n" //store 10,11

		"eor v30.16b, v30.16b, v16.16b \n"//buf[9] = f0g9^f1g8^f2g7^f3g6^f4g5^f5g4^f6g3^f7g2^f8g1^f9g0^(buf[21]^buf[18])
		"eor v29.16b, v29.16b, v17.16b \n"//buf[8] = f0g8^f1g7^f2g6^f3g5^f4g4^f5g3^f6g2^f7g1^f8g0^(buf[20]^buf[17])
		"stp q29, q30, [%[x0], #16*8] \n" //store 8,9

		"eor v28.16b, v28.16b, v18.16b \n"//buf[7] = f0g7^f1g6^f2g5^f3g4^f4g3^f5g2^f6g1^f7g0^(buf[19]^buf[16])
		"eor v27.16b, v27.16b, v19.16b \n"//buf[6] = f0g6^f0g6^f1g5^f2g4^f3g3^f4g2^f5g1^f6g0^(buf[18])
		"stp q27, q28, [%[x0], #16*6] \n" //store 6,7

		//"str q21, [%[x0], #16*12] \n"

		"and v14.16b, v12.16b, v5.16b \n"//tmp[5] = f5g0
		"and v15.16b, v12.16b, v4.16b \n"//tmp[4] = f4g0
		"and v16.16b, v12.16b, v3.16b \n"//tmp[3] = f3g0
		"eor v24.16b, v24.16b, v21.16b \n"//buf[3] = f0g3^f1g2^f2g1^(buf[15])
		"and v17.16b, v12.16b, v2.16b \n"//tmp[2] = f2g0
		"and v18.16b, v12.16b, v1.16b \n"//tmp[1] = f1g0
		"and v19.16b, v12.16b, v0.16b \n"//tmp[0] = f0g0

		"eor v26.16b, v26.16b, v14.16b \n"//buf[5] = f0g5^f1g4^f2g3^f3g2^f4g1^f5g0^(buf[17])
		"eor v25.16b, v25.16b, v15.16b \n"//buf[4] = f0g4^f1g3^f2g2^f3g1^f4g0^(buf[16])
		"stp q25, q26, [%[x0], #16*4] \n" //store 4,5

		"eor v24.16b, v24.16b, v16.16b \n"//buf[3] = f0g3^f1g2^f2g1^f3g0^(buf[15])
		"eor v23.16b, v23.16b, v17.16b \n"//buf[2] = f0g2^f1g1^f2g0^(buf[14])
		"stp q23, q24, [%[x0], #16*2] \n" //store 2,3

		"eor v22.16b, v22.16b, v18.16b \n"//buf[1] = f0g1^f1g0^(buf[13])
		"eor v21.16b, v21.16b, v19.16b \n"//buf[0] = f0g0^(buf[12])
		"stp q21, q22, [%[x0]] \n" //store 0,1

    : [x0] "+r" (h)
    : [x1] "r" (f), [x2] "r" (g)
    :	"q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "q16", "q17", "q18", "q19", "q20", "q21", "q22", "q23", "q24", "q25", "q26", "q27", "q28", "q29", "q30", "q31", "cc", "memory"
  );

}

/*
static inline void vec128_mul_neon(uint64x2_t *h, uint64x2_t *f, const uint64x2_t *g)
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



/*
static inline void vec128_mul(vec128 *h, vec128 *f, const vec128 *g)
{
	uint64_t t0 = ccnt_read();

	vec128_mul_asm(h, f, g);

	uint64_t t1 = ccnt_read();
  transpose128_time_count += t1-t0;
 	transpose128_count += 1;
}
*/




void vec128_sq(vec128 *, vec128 *);
void vec128_inv(vec128 *, vec128 *);

#endif
