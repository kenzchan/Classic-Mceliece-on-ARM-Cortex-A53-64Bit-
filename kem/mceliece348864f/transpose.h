/*
  This file is for matrix transposition
*/

#ifndef TRANSPOSE_H
#define TRANSPOSE_H
//#define transpose_64x128_sp_asm CRYPTO_NAMESPACE(transpose_64x128_sp_asm)
//#define transpose_64x64_asm CRYPTO_NAMESPACE(transpose_64x64_asm)

#include "vec128.h"

#include <stdint.h>

static inline void transpose_64x64(uint64_t * in) // 4094
{
	//uint32_t t0 = ccnt_read();

	int i, j;

	uint64_t tmp[64];

	uint64_t masks[3][2] = {
                        {0x5555555555555555, 0xAAAAAAAAAAAAAAAA},
                        {0x3333333333333333, 0xCCCCCCCCCCCCCCCC},
                        {0x0F0F0F0F0F0F0F0F, 0xF0F0F0F0F0F0F0F0},
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
  //uint32_t t1 = ccnt_read();
  //transpose128_time_count += t1-t0;
  //transpose128_count += 1;
}

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

#endif

