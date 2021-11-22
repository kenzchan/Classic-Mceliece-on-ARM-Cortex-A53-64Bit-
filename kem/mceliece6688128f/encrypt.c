/*
  This file is for Niederreiter encryption
*/

#include "encrypt.h"

#include "util.h"
#include "params.h"
#include "uint16_sort.h"
#include "randombytes.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "gf.h"

/* input: public key pk, error vector e */
/* output: syndrome s */
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

/* output: e, an error vector of weight t */
static void gen_e(unsigned char *e)
{
	int i, j, eq, count;

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
}

/* input: public key pk */
/* output: error vector e, syndrome s */
void encrypt(unsigned char *s, const unsigned char *pk, unsigned char *e)
{
	gen_e(e);

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

	syndrome(s, pk, e);
}

