/*
  This file is for matrix transposition
*/

#ifndef TRANSPOSE_H
#define TRANSPOSE_H

#include "vec128.h"

#include <stdio.h>
#include <stdint.h>

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
static inline void transpose_64x64(uint64_t * in) // 4094
{
	uint32_t t0 = ccnt_read();

	int i, j, s, d;

	uint64_t x, y;
	uint64_t masks[6][2] = {
	                        {0x5555555555555555, 0xAAAAAAAAAAAAAAAA},
	                        {0x3333333333333333, 0xCCCCCCCCCCCCCCCC},
	                        {0x0F0F0F0F0F0F0F0F, 0xF0F0F0F0F0F0F0F0},
	                        {0x00FF00FF00FF00FF, 0xFF00FF00FF00FF00},
	                        {0x0000FFFF0000FFFF, 0xFFFF0000FFFF0000},
	                        {0x00000000FFFFFFFF, 0xFFFFFFFF00000000}
	                       };

	for (d = 5; d >= 0; d--)
	{
		s = 1 << d;

		for (i = 0; i < 64; i += s*2)
		for (j = i; j < i+s; j++)
		{
			x = (in[j] & masks[d][0]) | ((in[j+s] & masks[d][0]) << s);
			y = ((in[j] & masks[d][1]) >> s) | (in[j+s] & masks[d][1]);

			in[j+0] = x;
			in[j+s] = y;
		}
	}

  uint32_t t1 = ccnt_read();
  transpose128_time_count += t1-t0;
  transpose128_count += 1;
}
*/

static void inline transpose_64x64(uint64x1_t * in) // 4094
{
	//uint32_t t0 = ccnt_read();

	int i, j;

	uint64x1_t tmp[64];

	uint64x1_t masks[3][2] = {
                        {{0x5555555555555555}, {0xAAAAAAAAAAAAAAAA}},
                        {{0x3333333333333333}, {0xCCCCCCCCCCCCCCCC}},
                        {{0x0F0F0F0F0F0F0F0F}, {0xF0F0F0F0F0F0F0F0}},
                      };

	// First round
	uint32x2x2_t tmp_32[32];
	uint16x4x2_t tmp_16[32];

	//for (i = 0; i < 64; i += s*2)
	for (i = 0; i < 32; i++)
	{
		tmp_32[i] = vtrn_u32(vreinterpret_u32_u64(in[i]), vreinterpret_u32_u64(in[i+32]));
	}

	// Second round
	//s >>= 1;
	for (i = 0; i < 16; i++)// s = 16
	{
		tmp_16[i] = vtrn_u16(vreinterpret_u16_u32(tmp_32[i].val[0]), vreinterpret_u16_u32(tmp_32[i+16].val[0]));
		tmp_16[i+16] = vtrn_u16(vreinterpret_u16_u32(tmp_32[i].val[1]), vreinterpret_u16_u32(tmp_32[i+16].val[1]));
	}

	// Third round
	for (i = 0; i < 8; i++)// s = 8
	{
		uint8x8x2_t tmp_8_0 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i].val[0]), vreinterpret_u8_u16(tmp_16[i+8].val[0]));
		uint8x8x2_t tmp_8_1 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i].val[1]), vreinterpret_u8_u16(tmp_16[i+8].val[1]));

		uint8x8x2_t tmp_8_2 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i].val[0]), vreinterpret_u8_u16(tmp_16[24+i].val[0]));
		uint8x8x2_t tmp_8_3 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i].val[1]), vreinterpret_u8_u16(tmp_16[24+i].val[1]));


		tmp[i] = vreinterpret_u64_u8(tmp_8_0.val[0]);
		tmp[8+i] = vreinterpret_u64_u8(tmp_8_0.val[1]);

		tmp[16+i] = vreinterpret_u64_u8(tmp_8_1.val[0]);
		tmp[24+i] = vreinterpret_u64_u8(tmp_8_1.val[1]);

		tmp[32+i] = vreinterpret_u64_u8(tmp_8_2.val[0]);
		tmp[40+i] = vreinterpret_u64_u8(tmp_8_2.val[1]);

		tmp[48+i] = vreinterpret_u64_u8(tmp_8_3.val[0]);
		tmp[56+i] = vreinterpret_u64_u8(tmp_8_3.val[1]);
	}
	
	//smaller than 8
	// 4-4 round
	//s = 1 << 2;
	for (i = 0; i < 64; i += 8)
	for (j = i; j < i+4; j++)
	{
		in[j] = (tmp[j] & masks[2][0]) | ((tmp[j+4] & masks[2][0]) << 4);
		in[j+4] = ((tmp[j] & masks[2][1]) >> 4) | (tmp[j+4] & masks[2][1]);
	}

	//s = 1 << 1;
	for (i = 0; i < 64; i += 4)
	for (j = i; j < i+2; j++)
	{
		tmp[j] = (in[j] & masks[1][0]) | ((in[j+2] & masks[1][0]) << 2);
		tmp[j+2] = ((in[j] & masks[1][1]) >> 2) | (in[j+2] & masks[1][1]);
	}

	//s = 1;
	for (i = 0; i < 64; i += 2)
	for (j = i; j < i+1; j++)
	{
		in[j] = (tmp[j] & masks[0][0]) | ((tmp[j+1] & masks[0][0]) << 1);
		in[j+1] = ((tmp[j] & masks[0][1]) >> 1) | (tmp[j+1] & masks[0][1]);
	}
}

/*
static inline void transpose_64x64(uint64_t * in)
{
	uint32_t t0 = ccnt_read();

	int i, j;

	uint64x2_t tmp[32];

	uint64x2_t tmpp[32];

	uint64_t masks[2] = {0x5555555555555555, 0xAAAAAAAAAAAAAAAA};

	// First round
	uint32x2x2_t tmp_32[32];
	uint16x4x2_t tmp_16[32];

	//for (i = 0; i < 64; i += s*2)
	for (i = 0; i < 32; i++)
	{
		tmp_32[i] = vtrn_u32(vreinterpret_u32_u64(in[i]), vreinterpret_u32_u64(in[i+32]));
	}

	// Second round
	//s >>= 1;
	for (i = 0; i < 16; i++)// s = 16
	{
		tmp_16[i] = vtrn_u16(vreinterpret_u16_u32(tmp_32[i].val[0]), vreinterpret_u16_u32(tmp_32[i+16].val[0]));
		tmp_16[i+16] = vtrn_u16(vreinterpret_u16_u32(tmp_32[i].val[1]), vreinterpret_u16_u32(tmp_32[i+16].val[1]));
	}

	// Third round
	for (i = 0; i < 8; i += 2)// s = 8
	{
		uint8x8x2_t tmp_8_0 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i].val[0]), vreinterpret_u8_u16(tmp_16[i+8].val[0]));
		uint8x8x2_t tmp_8_1 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i].val[1]), vreinterpret_u8_u16(tmp_16[i+8].val[1]));

		uint8x8x2_t tmp_8_2 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i].val[0]), vreinterpret_u8_u16(tmp_16[24+i].val[0]));
		uint8x8x2_t tmp_8_3 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i].val[1]), vreinterpret_u8_u16(tmp_16[24+i].val[1]));

		uint8x8x2_t tmp_8_4 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i+1].val[0]), vreinterpret_u8_u16(tmp_16[i+8+1].val[0]));
		uint8x8x2_t tmp_8_5 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i+1].val[1]), vreinterpret_u8_u16(tmp_16[i+8+1].val[1]));

		uint8x8x2_t tmp_8_6 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i+1].val[0]), vreinterpret_u8_u16(tmp_16[24+i+1].val[0]));
		uint8x8x2_t tmp_8_7 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i+1].val[1]), vreinterpret_u8_u16(tmp_16[24+i+1].val[1]));

		tmp[i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_0.val[0], tmp_8_4.val[0]));
		tmp[4+i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_0.val[1], tmp_8_4.val[1]));

		tmp[8+i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_1.val[0], tmp_8_5.val[0]));
		tmp[12+i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_1.val[1], tmp_8_5.val[1]));

		tmp[16+i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_2.val[0], tmp_8_6.val[0]));
		tmp[20+i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_2.val[1], tmp_8_6.val[1]));

		tmp[24+i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_3.val[0], tmp_8_7.val[0]));
		tmp[28+i/2] = vreinterpretq_u64_u8(vcombine_u8( tmp_8_3.val[1], tmp_8_7.val[1]));
	}

	vec128 maskd[2] = {{0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F}, {0xF0F0F0F0F0F0F0F0, 0xF0F0F0F0F0F0F0F0}};
	for (i = 0; i < 32; i += 2*2)
	for (j = i; j < i+2; j++)
	{
		tmpp[j+0] = (tmp[j] & maskd[0]) | ((tmp[j+2] & maskd[0]) << 4);
		tmpp[j+2] = ((tmp[j] & maskd[1]) >> 4) | (tmp[j+2] & maskd[1]);
	} 
	
	vec128 maskd2[2] = {{0x3333333333333333, 0x3333333333333333}, {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCC}};
	for (i = 0; i < 32; i += 2)
	for (j = i; j < i+1; j++){
		tmp[j+0] = (tmpp[j] & maskd2[0]) | ((tmpp[j+1] & maskd2[0]) << 2);
		tmp[j+1] = ((tmpp[j] & maskd2[1]) >> 2) | (tmpp[j+1] & maskd2[1]);
	}

	for (int j = 0; j < 32; j++){
		in[j*2+0] = (tmp[j][0] & masks[0]) | ((tmp[j][1] & masks[0]) << 1);
		in[j*2+1] = ((tmp[j][0] & masks[1]) >> 1) | (tmp[j][1] & masks[1]);
	}
  uint32_t t1 = ccnt_read();
  transpose128_time_count += t1-t0;
  transpose128_count += 1;
}
*/

	
  //t1 = ccnt_read();
  //fprintf(stderr, "%u\n", t1-t0);
/*
static inline void transpose_64x128_sp(vec128 * in) // pure bit mask
{
	uint32_t t0 = ccnt_read();

	int i, j, s, d;

	vec128 x, y;
	vec128 masks[6][2] = {
	                        {{0x5555555555555555, 0x5555555555555555}, {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA}},
	                        {{0x3333333333333333, 0x3333333333333333}, {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCC}},
	                        {{0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F}, {0xF0F0F0F0F0F0F0F0, 0xF0F0F0F0F0F0F0F0}},
	                        {{0x00FF00FF00FF00FF, 0x00FF00FF00FF00FF}, {0xFF00FF00FF00FF00, 0xFF00FF00FF00FF00}},
	                        {{0x0000FFFF0000FFFF, 0x0000FFFF0000FFFF}, {0xFFFF0000FFFF0000, 0xFFFF0000FFFF0000}},
	                        {{0x00000000FFFFFFFF, 0x00000000FFFFFFFF}, {0xFFFFFFFF00000000, 0xFFFFFFFF00000000}}
	                       };
	
	for (d = 5; d >= 0; d--)
	{
		s = 1 << d;

		for (i = 0; i < 64; i += s*2)
		for (j = i; j < i+s; j++)
		{
			x = (in[j] & masks[d][0]) | ((in[j+s] & masks[d][0]) << s);
			y = ((in[j] & masks[d][1]) >> s) | (in[j+s] & masks[d][1]);

			in[j+0] = x;
			in[j+s] = y;
		}
	}
  uint32_t t1 = ccnt_read();
  transpose128_time_count += t1-t0;
  transpose128_count += 1;
}
*/

/*
static inline void transpose_64x128_sp(vec128 * in) // Neon
{
	uint32_t t0 = ccnt_read();

	int i, j, s, d;

	vec128 x, y;
	vec128 masks[3][2] = {
	                        {{0x5555555555555555, 0x5555555555555555}, {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA}},
	                        {{0x3333333333333333, 0x3333333333333333}, {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCC}},
	                        {{0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F}, {0xF0F0F0F0F0F0F0F0, 0xF0F0F0F0F0F0F0F0}},
	                       };
	// First round
	uint32x2x2_t tmp_32a[32];
	uint32x2x2_t tmp_32b[32];

	uint16x4x2_t tmp_16a[32];
	uint16x4x2_t tmp_16b[32];

	for (i = 0; i < 32; i++)
	{
		tmp_32a[i] = vtrn_u32(vreinterpret_u32_u64(in[i][0]), vreinterpret_u32_u64(in[i+32][0]));
		tmp_32b[i] = vtrn_u32(vreinterpret_u32_u64(in[i][1]), vreinterpret_u32_u64(in[i+32][1]));
	}

	// Second round
	//s >>= 1;
	for (i = 0; i < 16; i++)// s = 16
	{
		tmp_16a[i] = vtrn_u16(vreinterpret_u16_u32(tmp_32a[i].val[0]), vreinterpret_u16_u32(tmp_32a[i+16].val[0]));
		tmp_16a[i+16] = vtrn_u16(vreinterpret_u16_u32(tmp_32a[i].val[1]), vreinterpret_u16_u32(tmp_32a[i+16].val[1]));

		tmp_16b[i] = vtrn_u16(vreinterpret_u16_u32(tmp_32b[i].val[0]), vreinterpret_u16_u32(tmp_32b[i+16].val[0]));
		tmp_16b[i+16] = vtrn_u16(vreinterpret_u16_u32(tmp_32b[i].val[1]), vreinterpret_u16_u32(tmp_32b[i+16].val[1]));
	}

	// Third round
	for (i = 0; i < 8; i++)// s = 8
	{
		uint8x8x2_t tmp_8_0a = vtrn_u8(vreinterpret_u8_u16(tmp_16a[i].val[0]), vreinterpret_u8_u16(tmp_16a[i+8].val[0]));
		uint8x8x2_t tmp_8_1a = vtrn_u8(vreinterpret_u8_u16(tmp_16a[i].val[1]), vreinterpret_u8_u16(tmp_16a[i+8].val[1]));

		uint8x8x2_t tmp_8_2a = vtrn_u8(vreinterpret_u8_u16(tmp_16a[16+i].val[0]), vreinterpret_u8_u16(tmp_16a[24+i].val[0]));
		uint8x8x2_t tmp_8_3a = vtrn_u8(vreinterpret_u8_u16(tmp_16a[16+i].val[1]), vreinterpret_u8_u16(tmp_16a[24+i].val[1]));

		uint8x8x2_t tmp_8_0b = vtrn_u8(vreinterpret_u8_u16(tmp_16b[i].val[0]), vreinterpret_u8_u16(tmp_16b[i+8].val[0]));
		uint8x8x2_t tmp_8_1b = vtrn_u8(vreinterpret_u8_u16(tmp_16b[i].val[1]), vreinterpret_u8_u16(tmp_16b[i+8].val[1]));

		uint8x8x2_t tmp_8_2b = vtrn_u8(vreinterpret_u8_u16(tmp_16b[16+i].val[0]), vreinterpret_u8_u16(tmp_16b[24+i].val[0]));
		uint8x8x2_t tmp_8_3b = vtrn_u8(vreinterpret_u8_u16(tmp_16b[16+i].val[1]), vreinterpret_u8_u16(tmp_16b[24+i].val[1]));


		in[i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_0a.val[0], tmp_8_0b.val[0]));
		in[8+i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_0a.val[1], tmp_8_0b.val[1]));

		in[16+i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_1a.val[0], tmp_8_1b.val[0]));
		in[24+i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_1a.val[1], tmp_8_1b.val[1]));

		in[32+i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_2a.val[0], tmp_8_2b.val[0]));
		in[40+i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_2a.val[1], tmp_8_2b.val[1]));

		in[48+i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_3a.val[0], tmp_8_3b.val[0]));
		in[56+i] = vreinterpretq_u64_u8(vcombine_u8(tmp_8_3a.val[1], tmp_8_3b.val[1]));
	}
	
	
	//smaller than 8
	
	for (d = 2; d >= 0; d--)
	{
		s = 1 << d;

		for (i = 0; i < 64; i += s*2)
		for (j = i; j < i+s; j++)
		{
			x = (in[j] & masks[d][0]) | ((in[j+s] & masks[d][0]) << s);
			y = ((in[j] & masks[d][1]) >> s) | (in[j+s] & masks[d][1]);

			in[j+0] = x;
			in[j+s] = y;
		}
	}
  uint32_t t1 = ccnt_read();
  transpose128_time_count += t1-t0;
  transpose128_count += 1;
}
*/


static inline void transpose_64x128_sp(vec128 * in) // Neon
{
	//uint32_t t0 = ccnt_read();

	int i, j, s, d;

	vec128 x, y;
	vec128 masks[3][2] = {
	                        {{0x5555555555555555, 0x5555555555555555}, {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA}},
	                        {{0x3333333333333333, 0x3333333333333333}, {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCC}},
	                        {{0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F}, {0xF0F0F0F0F0F0F0F0, 0xF0F0F0F0F0F0F0F0}},
	                       };
	// First round
	uint32x4x2_t tmp_32[32];
	uint16x8x2_t tmp_16[32];

	for (i = 0; i < 32; i++)
		{
			tmp_32[i] = vtrnq_u32(vreinterpretq_u32_u64(in[i]), vreinterpretq_u32_u64(in[i+32]));
		}

		// Second round
		//s >>= 1;
	for (i = 0; i < 16; i++)// s = 16
		{
			tmp_16[i] = vtrnq_u16(vreinterpretq_u16_u32(tmp_32[i].val[0]), vreinterpretq_u16_u32(tmp_32[i+16].val[0]));
			tmp_16[i+16] = vtrnq_u16(vreinterpretq_u16_u32(tmp_32[i].val[1]), vreinterpretq_u16_u32(tmp_32[i+16].val[1]));
		}

		// Third round
	for (i = 0; i < 8; i++)// s = 8
		{
			uint8x16x2_t tmp_8_0 = vtrnq_u8(vreinterpretq_u8_u16(tmp_16[i].val[0]), vreinterpretq_u8_u16(tmp_16[i+8].val[0]));
			uint8x16x2_t tmp_8_1 = vtrnq_u8(vreinterpretq_u8_u16(tmp_16[i].val[1]), vreinterpretq_u8_u16(tmp_16[i+8].val[1]));

			uint8x16x2_t tmp_8_2 = vtrnq_u8(vreinterpretq_u8_u16(tmp_16[16+i].val[0]), vreinterpretq_u8_u16(tmp_16[24+i].val[0]));
			uint8x16x2_t tmp_8_3 = vtrnq_u8(vreinterpretq_u8_u16(tmp_16[16+i].val[1]), vreinterpretq_u8_u16(tmp_16[24+i].val[1]));


			in[i] = vreinterpretq_u64_u8(tmp_8_0.val[0]);
			in[8+i] = vreinterpretq_u64_u8(tmp_8_0.val[1]);

			in[16+i] = vreinterpretq_u64_u8(tmp_8_1.val[0]);
			in[24+i] = vreinterpretq_u64_u8(tmp_8_1.val[1]);

			in[32+i] = vreinterpretq_u64_u8(tmp_8_2.val[0]);
			in[40+i] = vreinterpretq_u64_u8(tmp_8_2.val[1]);

			in[48+i] = vreinterpretq_u64_u8(tmp_8_3.val[0]);
			in[56+i] = vreinterpretq_u64_u8(tmp_8_3.val[1]);
		}
	
	//smaller than 8
	
	for (d = 2; d >= 0; d--)
	{
		s = 1 << d;

		for (i = 0; i < 64; i += s*2)
		for (j = i; j < i+s; j++)
		{
			x = (in[j] & masks[d][0]) | ((in[j+s] & masks[d][0]) << s);
			y = ((in[j] & masks[d][1]) >> s) | (in[j+s] & masks[d][1]);

			in[j+0] = x;
			in[j+s] = y;
		}
	}
}


/*
static inline void transpose_64x128_sp(vec128 * in) // Neon
{
	//uint32_t t0 = ccnt_read();

	int i, j, s, d;

	vec128 x, y;
	vec128 masks[3][2] = {
	                        {{0x5555555555555555, 0x5555555555555555}, {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA}},
	                        {{0x3333333333333333, 0x3333333333333333}, {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCC}},
	                        {{0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F}, {0xF0F0F0F0F0F0F0F0, 0xF0F0F0F0F0F0F0F0}},
	                       };
	// First round
	uint32x2x2_t tmp_32[32];
	uint16x4x2_t tmp_16[32];

	for (j = 0; j < 2; j ++){
		for (i = 0; i < 32; i++)
		{
			tmp_32[i] = vtrn_u32(vreinterpret_u32_u64(in[i][j]), vreinterpret_u32_u64(in[i+32][j]));
		}

		// Second round
		//s >>= 1;
		for (i = 0; i < 16; i++)// s = 16
		{
			tmp_16[i] = vtrn_u16(vreinterpret_u16_u32(tmp_32[i].val[0]), vreinterpret_u16_u32(tmp_32[i+16].val[0]));
			tmp_16[i+16] = vtrn_u16(vreinterpret_u16_u32(tmp_32[i].val[1]), vreinterpret_u16_u32(tmp_32[i+16].val[1]));
		}

		// Third round
		for (i = 0; i < 8; i++)// s = 8
		{
			uint8x8x2_t tmp_8_0 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i].val[0]), vreinterpret_u8_u16(tmp_16[i+8].val[0]));
			uint8x8x2_t tmp_8_1 = vtrn_u8(vreinterpret_u8_u16(tmp_16[i].val[1]), vreinterpret_u8_u16(tmp_16[i+8].val[1]));

			uint8x8x2_t tmp_8_2 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i].val[0]), vreinterpret_u8_u16(tmp_16[24+i].val[0]));
			uint8x8x2_t tmp_8_3 = vtrn_u8(vreinterpret_u8_u16(tmp_16[16+i].val[1]), vreinterpret_u8_u16(tmp_16[24+i].val[1]));


			in[i][j] = vreinterpret_u64_u8(tmp_8_0.val[0]);
			in[8+i][j] = vreinterpret_u64_u8(tmp_8_0.val[1]);

			in[16+i][j] = vreinterpret_u64_u8(tmp_8_1.val[0]);
			in[24+i][j] = vreinterpret_u64_u8(tmp_8_1.val[1]);

			in[32+i][j] = vreinterpret_u64_u8(tmp_8_2.val[0]);
			in[40+i][j] = vreinterpret_u64_u8(tmp_8_2.val[1]);

			in[48+i][j] = vreinterpret_u64_u8(tmp_8_3.val[0]);
			in[56+i][j] = vreinterpret_u64_u8(tmp_8_3.val[1]);
		}
	}
	
	
	//smaller than 8
	
	for (d = 2; d >= 0; d--)
	{
		s = 1 << d;

		for (i = 0; i < 64; i += s*2)
		for (j = i; j < i+s; j++)
		{
			x = (in[j] & masks[d][0]) | ((in[j+s] & masks[d][0]) << s);
			y = ((in[j] & masks[d][1]) >> s) | (in[j+s] & masks[d][1]);

			in[j+0] = x;
			in[j+s] = y;
		}
	}
  //uint32_t t1 = ccnt_read();
  //transpose128_time_count += t1-t0;
  //transpose128_count += 1;
}
*/



#endif
















