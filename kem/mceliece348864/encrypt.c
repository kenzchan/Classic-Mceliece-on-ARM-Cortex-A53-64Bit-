//#define syndrome_asm CRYPTO_NAMESPACE(syndrome_asm)
//#define _syndrome_asm _CRYPTO_NAMESPACE(syndrome_asm)
/*
  This file is for Niederreiter encryption
*/

#include "encrypt.h"

#include "gf.h"
#include "util.h"
#include "params.h"
#include "uint16_sort.h"
#include "randombytes.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* input: public key pk, error vector e */
/* output: syndrome s */
//extern void syndrome_asm(unsigned char *s, const unsigned char *pk, unsigned char *e);

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



static void syndrome(unsigned char *s, const unsigned char *pk, unsigned char *e) //294219
{
	int i, j;
	uint32x4_t tmp1;
	uint16x8_t tmp2;
	vec128 tmp4;
	uint64_t tmp5;

	const uint32_t *e_prt = ((uint32_t *)(e + SYND_BYTES));

	for (i = 0; i < SYND_BYTES; i++)
		s[i] = e[i];

	for (i = 0; i < PK_NROWS; i++)	
	{

		const uint32_t *pk_prt = ((uint32_t *)(pk + PK_ROW_BYTES * i));
		tmp1 = vandq_u32(vld1q_u32(e_prt), vld1q_u32(pk_prt));


		for (j = 1; j < PK_NCOLS/128; j++){

			uint32x4_t e = vld1q_u32(e_prt + 4*j);
			uint32x4_t p = vld1q_u32(pk_prt + 4*j);

			tmp1 = veorq_u32(tmp1, vandq_u32(e, p));
		}

		tmp1[3] ^= (pk_prt + 4*j)[0] & (e_prt + 4*j)[0];

		tmp2 = vpaddlq_u8(vcntq_u8(vreinterpretq_u8_u32(tmp1)));
		tmp1 = vpaddlq_u16(tmp2);
		tmp4 = vpaddlq_u32(tmp1);
		tmp5 = vget_low_u64(tmp4)^vget_high_u64(tmp4);

		s[ i/8 ] ^= ((tmp5 & 1) << (i%8));
	}
}

/*
static void syndrome(unsigned char *s, const unsigned char *pk, unsigned char *e) //290024
{
	int i, j;
	uint32x4_t tmp1;
	uint16x8_t tmp2;
	vec128 tmp4;
	uint64_t tmp5;

	const uint32_t *e_prt = ((uint32_t *)(e + SYND_BYTES));

	for (i = 0; i < SYND_BYTES; i++)
		s[i] = e[i];

	for (i = 0; i < PK_NROWS; i++)	
	{

		const uint32_t *pk_prt = ((uint32_t *)(pk + PK_ROW_BYTES * i));
		tmp1 = vandq_u32(vld1q_u32(e_prt), vld1q_u32(pk_prt));


		for (j = 1; j < PK_NCOLS/128; j++){

			uint32x4_t e = vld1q_u32(e_prt + 4*j);
			uint32x4_t p = vld1q_u32(pk_prt + 4*j);

			tmp1 = veorq_u32(tmp1, vandq_u32(e, p));
		}

		tmp1[3] ^= (pk_prt + 4*j)[0] & (e_prt + 4*j)[0];

		tmp2 = vpaddlq_u8(vcntq_u8(vreinterpretq_u8_u32(tmp1)));
		tmp1 = vpaddlq_u16(tmp2);
		tmp4 = vpaddlq_u32(tmp1);
		tmp5 = vget_low_u64(tmp4)^vget_high_u64(tmp4);

		s[ i/8 ] ^= ((tmp5 & 1) << (i%8));
	}
}
*/

/*
static void syndrome(unsigned char *s, const unsigned char *pk, unsigned char *e) //297017
{
	int i, j;
	uint8x8_t tmp1;
	uint16x4_t tmp2;
	uint32x2_t tmp3;
	uint64_t tmp4;

	const unsigned char *e_prt = ((unsigned char *)(e + SYND_BYTES));

	for (i = 0; i < SYND_BYTES; i++)
		s[i] = e[i];

	for (i = 0; i < PK_NROWS; i++)	
	{
		const unsigned char *pk_prt = ((unsigned char *)(pk + PK_ROW_BYTES * i));
		tmp1 = vand_u8(vld1_u8((e_prt)), vld1_u8((pk_prt)));

		for (j = 1; j < PK_NCOLS/64; j++){
			tmp1 = veor_u8(tmp1, vand_u8(vld1_u8((e_prt + 8*j)), vld1_u8((pk_prt + 8*j))));
		}

		tmp3 = vreinterpret_u32_u8(tmp1);
		tmp3[1] ^= ((uint32_t *) (pk_prt + 8*j))[0] & ((uint32_t *) (e_prt + 8*j))[0];

		tmp2 = vpaddl_u8(vcnt_u8(vreinterpret_u8_u32(tmp3)));
		tmp3 = vpaddl_u16(tmp2);
		tmp4 = vpaddl_u32(tmp3);

		s[ i/8 ] ^= ((tmp4&1) << (i%8));
	}
}
*/


/* input: public key pk, error vector e */
/* output: syndrome s */
/*
static void syndrome(unsigned char *s, const unsigned char *pk, unsigned char *e)
{
	unsigned char b, row[SYS_N/8];
	const unsigned char *pk_ptr = pk;

	int i, j;

	for (i = 0; i < SYND_BYTES; i++)
		s[i] = 0;

	for (i = 0; i < PK_NROWS; i++)	
	{
		for (j = 0; j < SYS_N/8; j++) 
			row[j] = 0;

		for (j = 0; j < PK_ROW_BYTES; j++) 
			row[ SYS_N/8 - PK_ROW_BYTES + j ] = pk_ptr[j];

		row[i/8] |= 1 << (i%8);
		
		b = 0;
		for (j = 0; j < SYS_N/8; j++)
			b ^= row[j] & e[j];

		b ^= b >> 4;
		b ^= b >> 2;
		b ^= b >> 1;
		b &= 1;

		s[ i/8 ] |= (b << (i%8));

		pk_ptr += PK_ROW_BYTES;
	}
}
*/




/* output: e, an error vector of weight t */
static void gen_e(unsigned char *e) //1223
{
	int i, j, eq, count;

	//fprintf(stderr, "Error union1\n");

	union 
	{
		uint16_t nums[ SYS_T*2 ];
		unsigned char bytes[ SYS_T*2 * sizeof(uint16_t) ];
	} buf;

	uint16_t ind[ SYS_T ];
	uint64_t e_int[ (SYS_N+63)/64 ];	
	uint64_t one = 1;	
	uint64_t mask;	
	uint64_t val[ SYS_T ];	

	while (1)
	{
		randombytes(buf.bytes, sizeof(buf));

		for (i = 0; i < SYS_T*2; i++)
			buf.nums[i] = load_gf(buf.bytes + i*2);

		// moving and counting indices in the correct range
		count = 0;
		for (i = 0; i < SYS_T*2 && count < SYS_T; i++)
			if (buf.nums[i] < SYS_N)
				ind[ count++ ] = buf.nums[i];
		
		if (count < SYS_T) continue;

		// check for repetition
		uint16_sort(ind, SYS_T);

		eq = 0;
		for (i = 1; i < SYS_T; i++)
			if (ind[i-1] == ind[i])
				eq = 1;

		if (eq == 0)
			break;
	}

	for (j = 0; j < SYS_T; j++)
		val[j] = one << (ind[j] & 63);

	for (i = 0; i < (SYS_N+63)/64; i++) 
	{
		e_int[i] = 0;

		for (j = 0; j < SYS_T; j++)
		{
			mask = i ^ (ind[j] >> 6);
			mask -= 1;
			mask >>= 63;
			mask = -mask;

			e_int[i] |= val[j] & mask;
		}
	}

	for (i = 0; i < (SYS_N+63)/64 - 1; i++) 
		{ store8(e, e_int[i]); e += 8; }

	for (j = 0; j < (SYS_N % 64); j+=8) 
		e[ j/8 ] = (e_int[i] >> j) & 0xFF;

	//fprintf(stderr, "Error outgen\n");

}

void encrypt(unsigned char *s, const unsigned char *pk, unsigned char *e)
{
	//uint32_t t0 = ccnt_read();
	gen_e(e);
	//uint32_t t1 = ccnt_read();
  //transpose128_time_count += t1-t0;
  //transpose128_count += 1;

  //fprintf(stderr, "sss %u \n", transpose128_time_count);

#ifdef KAT
  {
    int k;
    printf("encrypt e: positions");
    for (k = 0;k < SYS_N;++k)
      if (e[k/8] & (1 << (k&7)))
        printf(" %d",k);
    printf("\n");
  }
#endif
	//uint32_t t0 = ccnt_read();
	syndrome(s, pk, e);
	//uint32_t t1 = ccnt_read();
  //transpose128_time_count += t1-t0;
  //transpose128_count += 1;

  //fprintf(stderr, "sss %u \n", transpose128_time_count);

	//fprintf(stderr, "Error after_syndrome\n");

}

