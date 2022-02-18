/*
  This file is for transpose of the Gao-Mateer FFT
*/

#ifndef FFT_TR_H
#define FFT_TR_H
#define fft_tr CRYPTO_NAMESPACE(fft_tr)
#define print128_num CRYPTO_NAMESPACE(print128_num)


#include "params.h"
#include "vec128.h"

void fft_tr(vec128 [GFBITS], vec128 [][ GFBITS ]);
void print128_num(vec128 var);

#endif

