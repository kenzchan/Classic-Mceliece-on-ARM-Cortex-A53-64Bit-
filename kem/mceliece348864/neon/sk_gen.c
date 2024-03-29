/*
  This file is for secret-key generation
*/

#include "sk_gen.h"

#include "randombytes.h"
#include "controlbits.h"
#include "transpose.h"
#include "params.h"
#include "util.h"
#include "vec.h"
#include "gf.h"

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

/* input: v, a list of GF(2^m) elements in bitsliced form */
/* input: idx, an index */
/* return: the idx-th element in v */
static inline gf extract_gf(vec v[GFBITS], int idx)
{
	int i;
	gf ret;	

	ret = 0;
	for (i = GFBITS-1; i >= 0; i--)
	{
		ret <<= 1;
		ret |= (v[i] >> idx) & 1;
	}

	return ret;
}

/* same as extract_gf but reduces return value to 1 bit */
static inline vec extract_bit(vec v[GFBITS], int idx)
{
	int i;
	vec ret;	

	ret = 0;
	for (i = GFBITS-1; i >= 0; i--)
		ret |= v[i];

	return (ret >> idx) & 1;
}

/* input: f, element in GF((2^m)^t) */
/* output: out, minimal polynomial of f */
/* return: 0 for success and -1 for failure */
int genpoly_gen(gf *out, gf *f)
{
	int i, j, k;

	gf t, inv;

	vec v[ GFBITS ], mat[ 64 ][ GFBITS ], mask;

	union 
	{
		uint64_t c[ GFBITS ][ 64 ];
		uint64x1_t neon[ GFBITS ][ 64 ];
	} buf;

	// fill matrix

	buf.c[0][0] = 1;
	for (i = 1; i < GFBITS; i++)
		buf.c[i][0] = 0;

	for (j = 0; j < GFBITS; j++)
	for (i = SYS_T-1; i >= 0; i--)
	{
		v[j] <<= 1;
		v[j] |= (f[i] >> j) & 1;
	}
	
	for (i = 0; i < GFBITS; i++)
		buf.c[i][1] = v[i];		

	for (k = 2; k <= SYS_T; k++)
	{
		vec_GF_mul(v, v, f);

		if (k < SYS_T)
		{
			for (i = 0; i < GFBITS; i++)
				buf.c[i][k] = v[i];		
		}
		else
		{
			for (i = 0; i < SYS_T; i++)
				out[i] = extract_gf(v, i);
		}
	}

	for (i = 0; i < GFBITS; i++){
		//uint32_t t0 = ccnt_read();
		transpose_64x64(buf.neon[i]);
		//uint32_t t1 = ccnt_read();
  	//transpose128_time_count += t1-t0;
  	//transpose128_count += 1;
  	}
	
	for (j = 0; j < SYS_T; j++)
	for (i = 0; i < GFBITS; i++)
		mat[j][i] = buf.c[i][j];

	// gaussian

	for (i = 0; i < SYS_T; i++)
	{
		for (j = i+1; j < SYS_T; j++)
		{
			mask = extract_bit(mat[i], i);
			mask -= 1;

			for (k = 0; k < GFBITS; k++)
				mat[i][k] ^= mat[j][k] & mask;

			out[i] ^= out[j] & mask;
		}

		//

		t = extract_gf(mat[i], i);

		if (t == 0) return -1; // return if not systematic

		//

		inv = gf_inv(t);
		vec_mul_gf(mat[i], mat[i], inv);

		out[i] = gf_mul(out[i], inv);

		for (j = 0; j < SYS_T; j++)
		{
			if (j != i)
			{
				t = extract_gf(mat[j], i);

				vec_mul_gf(v, mat[i], t);
				vec_add(mat[j], mat[j], v);

				out[j] ^= gf_mul(out[i], t);
			}
		}
	}	

	return 0;
}

