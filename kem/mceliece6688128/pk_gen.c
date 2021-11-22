/*
  This file is for public-key generation
*/

#include "pk_gen.h"

#include "controlbits.h"
#include "uint64_sort.h"
#include "int32_sort.h"
#include "params.h"
#include "util.h"
#include "fft.h"

#include <stdint.h>

#define par_width 20

/* set m to 11...1 if the i-th bit of x is 0 and the i-th bit of y is 1 */
/* set m to 00...0 otherwise */
static inline void extract_01_masks(uint16_t *m, uint64_t *x, uint64_t *y, int i)
{
	*m = (((~x[ i>>6 ]) & y[ i>>6 ]) >> (i&63)) & 1;
	*m = -(*m);
}

/* return a 128-bit vector of which each bits is set to the i-th bit of v */
static inline vec128 extract_mask128(uint64_t v[], int i)
{
	uint32_t mask;

	mask = (v[ i>>6 ] >> (i&63)) & 1;
	mask = -mask;

	return vec128_set1_32b(mask);
}

/* swap x and y if m = 11...1 */
static inline void uint16_cswap(uint16_t *x, uint16_t *y, uint16_t m)
{
	uint16_t d;

	d = *x ^ *y;
	d &= m;
	*x ^= d;
	*y ^= d;
}

/* swap x and y if m = 11...1 */
static inline void vec128_cswap(vec128 *x, vec128 *y, vec128 m)
{
	vec128 d;

	d = *x ^ *y;
	d &= m;
	*x ^= d;
	*y ^= d;
}

/* swap   x[i0] and   x[i1]  if x[i1] > x[i0] */
/* swap mat[i0] and mat[i1]  if x[i1] > x[i0] */
static inline void minmax_rows(uint16_t *x, vec128 (*mat)[par_width], int i0, int i1)
{
	int i;
	uint16_t m;
	vec128 mm;

	m = x[i1] - x[i0];
	m >>= 15;
	m = -m;
	mm = vec128_set1_16b(m);

	uint16_cswap(&x[i0], &x[i1], m);

	for (i = 0; i < par_width; i++)
		vec128_cswap(&mat[i0][i], &mat[i1][i], mm);
}

/* merge first half of x[0],x[step],...,x[(2*n-1)*step] with second half */
/* requires n to be a power of 2 */
static void merge_rows(int n, int bound, uint16_t *x, vec128 (*mat)[par_width], int off, int step)
{
	int i;

	if (n == 1) 
	{
		if(off + step < bound)
			minmax_rows(x, mat, off, off + step);
	}
	else 
	{
		merge_rows(n/2, bound, x, mat, off, step*2);
		merge_rows(n/2, bound, x, mat, off + step, step*2);

		for (i = 1; i < 2*n-1 && off + (i+1) * step < bound; i += 2)
			minmax_rows(x, mat, off + i*step, off + (i+1)*step);
	}
}

/* permute the rows of mat by sorting x */
static void sort_rows(int n, int bound, uint16_t *x, vec128 (*mat)[par_width], int off)
{
	if (n <= 1) return;
	sort_rows(n/2, bound, x, mat, off);
	sort_rows(n/2, bound, x, mat, off + n/2);
	merge_rows(n/2, bound, x, mat, off, 1);
}

/* extract numbers represented in bitsliced form */
static void de_bitslicing(uint64_t * out, const vec128 in[][GFBITS])
{
	int i, j, r;
	uint64_t u = 0;

	for (i = 0; i < (1 << GFBITS); i++)
		out[i] = 0 ;

	for (i = 0; i < 64; i++)
	for (j = GFBITS-1; j >= 0; j--) {
		u = vec128_extract(in[i][j], 0);
		for (r = 0; r < 64; r++)
		{
			out[i*128 + 0*64 + r] <<= 1;
			out[i*128 + 0*64 + r] |= (u >> r) & 1;
		}
		u = vec128_extract(in[i][j], 1);
		for (r = 0; r < 64; r++)
		{
			out[i*128 + 1*64 + r] <<= 1;
			out[i*128 + 1*64 + r] |= (u >> r) & 1;
		}
	}
}

/* convert numbers into bitsliced form */
static void to_bitslicing_2x(vec128 out0[][GFBITS], vec128 out1[][GFBITS], const uint64_t * in)
{
	int i, j, k, r;
	uint64_t u[2][2];

	for (i = 0; i < 64; i++)
	for (j = GFBITS-1; j >= 0; j--)
	{
		for (k = 0; k < 2; k++)
		for (r = 63; r >= 0; r--)
		{
			u[0][k] <<= 1;
			u[0][k] |= (in[i*128 + k*64 + r] >> (GFBITS-1-j)) & 1;

			u[1][k] <<= 1;
			u[1][k] |= (in[i*128 + k*64 + r] >> (j + GFBITS)) & 1;
		}
    
		out0[i][j] = vec128_set2x(u[0][0], u[0][1]);
		out1[i][j] = vec128_set2x(u[1][0], u[1][1]);
	}
}

/* y[pi[i]] = x[i] */
/* requires pi to be a permutation */
static void composeinv(int n, uint16_t y[n], uint16_t x[n], uint16_t pi[n])
{
  int i;
  int32_t t[n];

  for (i = 0;i < n;++i) 
  {
    t[i] = pi[i];
    t[i] <<= 16;
    t[i] |= x[i];
  }

  int32_sort(t,n);

  for (i = 0;i < n;++i)
    y[i] = t[i] & 0xFFFF;
}

/* input: irr, an irreducible polynomial */
/*        perm, a permutation represented as an array of 32-bit numbers */
/*        pi, same permutation represented as an array of 16-bit numbers */
/* output: pk, the public key*/
/* return: 0 if pk is successfully generated, -1 otherwise */
int pk_gen(unsigned char * pk, const unsigned char * irr, uint32_t * perm, int16_t * pi)
{
	const int nBlocks_I = (PK_NROWS + 127) / 128;

	int i, j, k, b;
	int row, c;
		
	union
	{
		uint64_t w[ PK_NROWS ][ nBlocks_I*2 ];
		vec128   v[ PK_NROWS ][ nBlocks_I ];
	} mat;

	union
	{
		 uint64_t w[ PK_NROWS ][ par_width*2 ];
		 vec128 v[ PK_NROWS ][ par_width ];
	} par;

	uint16_t m;	
	vec128 mm;

	vec128 sk_int[ GFBITS ];

	vec128 consts[ 64 ][ GFBITS ];
	vec128 eval[ 64 ][ GFBITS ];
	vec128 prod[ 64 ][ GFBITS ];
	vec128 tmp[ GFBITS ];

	uint64_t list[1 << GFBITS];
	uint64_t one = 1;
	uint64_t t;

	uint16_t ind[ (1 << GFBITS)/4 ];
	uint16_t ind_inv[ (1 << GFBITS)/4 ];

	// compute the inverses 

	irr_load(sk_int, irr);

	fft(eval, sk_int);

	vec128_copy(prod[0], eval[0]);

	for (i = 1; i < 64; i++)
		vec128_mul(prod[i], prod[i-1], eval[i]);

	vec128_inv(tmp, prod[63]);

	for (i = 62; i >= 0; i--)
	{
		vec128_mul(prod[i+1], prod[i], tmp);
		vec128_mul(tmp, tmp, eval[i+1]);
	}

	vec128_copy(prod[0], tmp);

	// fill matrix 

	de_bitslicing(list, prod);

	for (i = 0; i < (1 << GFBITS); i++)
	{	
		list[i] <<= GFBITS;
		list[i] |= i;	
		list[i] |= ((uint64_t) perm[i]) << 31;
	}

	uint64_sort(list, 1 << GFBITS);

	for (i = 1; i < (1 << GFBITS); i++)
		if ((list[i-1] >> 31) == (list[i] >> 31))
			return -1;

	to_bitslicing_2x(consts, prod, list);

	for (i = 0; i < (1 << GFBITS); i++)
		pi[i] = list[i] & GFMASK;

	for (j = 0; j < nBlocks_I; j++)
	for (k = 0; k < GFBITS; k++)
		mat.v[ k ][ j ] = prod[ j ][ k ];

	for (i = 1; i < SYS_T; i++)
	for (j = 0; j < nBlocks_I; j++)
	{
		vec128_mul(prod[j], prod[j], consts[j]);

		for (k = 0; k < GFBITS; k++)
			mat.v[ i*GFBITS + k ][ j ] = prod[ j ][ k ];
	}

	// gaussian elimination to obtain L, U, and P such that LP M = U
	// L and U are stored in the space of M
	// P is stored in ind

	for (i = 0; i < (1 << GFBITS)/4; i++)
		ind_inv[i] = ind[ i ] = i;

	for (row = 0; row < PK_NROWS; row++)
	{
		i = row >> 6;
		j = row & 63;

		for (k = row + 1; k < PK_NROWS; k++)
		{
			extract_01_masks(&m, mat.w[ row ], mat.w[ k ], row);
			uint16_cswap(&ind[row], &ind[k], m);

			mm = vec128_set1_16b(m);
			for (c = 0; c < nBlocks_I; c++)
				vec128_cswap(&mat.v[ row ][ c ], &mat.v[ k ][ c ], mm);
		}

		if ( ((mat.w[ row ][ i ] >> j) & 1) == 0 ) // return if not systematic
		{
			return -1;
		}

		for (k = row+1; k < PK_NROWS; k++)
		{
			t = mat.w[ k ][ i ] & (one << j);
			mm = extract_mask128(mat.w[k], row);

			for (c = 0; c < nBlocks_I; c++)
				mat.v[ k ][ c ] ^= mat.v[ row ][ c ] & mm;

			mat.w[ k ][ i ] |= t;
		}
	}

	// apply M^-1 to the remaining columns

	composeinv((1 << GFBITS)/4, ind_inv, ind_inv, ind);

	for (j = 13; j < 53; j += par_width)
	{
		for (k = 0; k < GFBITS; k++)
		for (b = 0; b < par_width; b++) 
			par.v[ k ][ b ] = prod[ j+b ][ k ];
        
		for (i = 1; i < SYS_T; i++)
		{
			for (b = 0; b < par_width; b++) 
				vec128_mul(prod[ j+b ], prod[ j+b ], consts[ j+b ]);
        
			for (k = 0; k < GFBITS; k++)
			for (b = 0; b < par_width; b++) 
				par.v[ i*GFBITS + k ][ b ] = prod[ j+b ][ k ];
		}

		// apply P

		for (i = 0; i < PK_NROWS; i++)
			ind[i] = ind_inv[i];

		sort_rows((1 << GFBITS)/4, PK_NROWS, ind, par.v, 0);

		// apply L
   
		for (row = PK_NROWS-1; row >= 0; row--)
		for (i = row-1; i >= 0; i--)
		{
			mm = extract_mask128(mat.w[row], i);
    
			for (k = 0; k < par_width; k++)
				par.v[ row ][ k ] ^= par.v[ i ][ k ] & mm;
		}
    
		// apply U^-1

		for (row = PK_NROWS-1; row >= 0; row--)
		for (i = PK_NROWS-1; i > row; i--)
		{
			mm = extract_mask128(mat.w[row], i);
    
			for (k = 0; k < par_width; k++)
				par.v[ row ][ k ] ^= par.v[ i ][ k ] & mm;
		}

		if (j == 13)
		{
			for (row = 0; row < PK_NROWS; row++)
			for (k = 0; k < 40; k++)
				store8(pk + PK_ROW_BYTES * row + j*16 + k*8 - 13*16, par.w[row][k]);
		}
		else
		{
			for (row = 0; row < PK_NROWS; row++)
			{
				for (k = 0; k < 38; k++)
					store8(pk + PK_ROW_BYTES * row + j*16 + k*8 - 13*16, par.w[row][k]);

				store_i(pk + PK_ROW_BYTES * row + j*16 + k*8 - 13*16, par.w[row][k], 4);
			}
		}
	}

	//

	return 0;
}
