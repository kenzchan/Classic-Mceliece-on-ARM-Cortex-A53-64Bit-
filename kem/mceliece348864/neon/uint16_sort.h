#ifndef uint16_sort_h
#define uint16_sort_h

#define uint16_sort CRYPTO_NAMESPACE(uint16_sort)

#include <inttypes.h>
#include "params.h"

/* Macros */

/* In-place swap top 64 bits of “a” with bottom 64 bits of “b” -- one operation (vswp) */
#define vtrn64q(a, b) \
do { \
uint16x4_t vtrn64_tmp_a0 = vget_low_u16(a), vtrn64_tmp_a1 = vget_high_u16(a); \
uint16x4_t vtrn64_tmp_b0 = vget_low_u16(b), vtrn64_tmp_b1 = vget_high_u16(b); \
{\
uint16x4_t vtrn64_tmp = vtrn64_tmp_a1; \
vtrn64_tmp_a1 = vtrn64_tmp_b0; \
vtrn64_tmp_b0 = vtrn64_tmp;\
}\
(a) = vcombine_u16(vtrn64_tmp_a0, vtrn64_tmp_a1); \
(b) = vcombine_u16(vtrn64_tmp_b0, vtrn64_tmp_b1); \
} while (0)

/* In-place exchange odd 32-bit words of “a” with even 32-bit words of “b” -- one operation
*/
#define vtrn32q(a, b) \
do { \
uint32x4x2_t vtrn32_tmp = \
vtrnq_u32(vreinterpretq_u32_u16(a), vreinterpretq_u32_u16(b)); \
(a) = vreinterpretq_u16_u32(vtrn32_tmp.val[0]); \
(b) = vreinterpretq_u16_u32(vtrn32_tmp.val[1]); \
} while (0)

#define vtrn32(a, b) do { \
uint32x2x2_t vtrn32_tmp = vtrn_u32(vreinterpret_u32_u16(a),vreinterpret_u32_u16(b));\
(a) = vreinterpret_u16_u32(vtrn32_tmp.val[0]); \
(b) = vreinterpret_u16_u32(vtrn32_tmp.val[1]); \
} while (0)

/* In-place exchange odd 16-bit words of “a” with even 16-bit words of “b” -- one operation */
#define vtrn16q(a, b) \
do { \
uint16x8x2_t vtrn16_tmp = vtrnq_u16((a), (b)); \
(a) = vtrn16_tmp.val[0]; \
(b) = vtrn16_tmp.val[1]; \
} while (0)

#define vzipq(a, b) \
do { \
uint16x8x2_t vzip_tmp = vzipq_u16((a), (b)); \
(a) = vzip_tmp.val[0]; \
(b) = vzip_tmp.val[1]; \
} while (0)

#define vminmaxq(a, b) \
do { \
uint16x8_t minmax_tmp = (a); \
(a) = vminq_u16((a), (b)); \
(b) = vmaxq_u16(minmax_tmp, (b)); \
} while (0)

#define vrev128q_u16(a) \
vcombine_u16(vrev64_u16(vget_high_u16(a)), vrev64_u16(vget_low_u16(a)))

/*
static inline void bitonic_resort_16(uint16_t * ptr, uint16x8_t a, uint16x8_t b) 
{
  vtrn64q(a ,b);
  vminmaxq(a, b);

  vtrn32q(a, b);
  vminmaxq(a, b);

  vtrn16q(a, b);
  vminmaxq(a, b);

  uint16x8x2_t num = {a, b};
  vst2q_u16(ptr, num);
  //return vzipq_u16(a, b);
}


static inline void bitonic_merge_16(uint16_t * ptr, uint16x8_t a, uint16x8_t b)
{
  //order 2
  vminmaxq(a, b);
  vzipq(a, b);

  //order 4
  //vtrn32q(a, b);
  b = vrev32q_u16(b);
  vminmaxq(a, b);
  vtrn16q(a, b);
  vminmaxq(a, b);
  vzipq(a, b);

  //order 8
  //vtrn64q(a, b);
  b = vrev64q_u16(b);
  vminmaxq(a, b);
  vtrn32q(a, b);
  vminmaxq(a, b);
  vtrn16q(a, b);
  vminmaxq(a, b);
  vzipq(a, b);

  //order16
  b = vrev128q_u16(b); 
  vminmaxq(a, b);
  bitonic_resort_16(ptr, a, b);
}

static inline int merge(uint16_t *tmp, uint16_t *L, uint16_t *R)
{
  int i = 0; // Initial index of first subarray
  int j = 0; // Initial index of second subarray
  int k = 0; // Initial index of merged subarray

  while (i < 16 && j < 16) {
      if (L[i] <= R[j]) {
          if (k > 0 && tmp[k - 1] == L[i]){
            return 1;
          }
          tmp[k] = L[i];
          i++;
      }
      else {
          if (k > 0 && tmp[k - 1] == R[j]){
            return 1;
          }
          tmp[k] = R[j];
          j++;
      }
      k++;
  }


  while (i < 16) {
      if (k > 0 && tmp[k - 1] == L[i])
        return 1;
      tmp[k] = L[i];
      i++;
      k++;
  }


  while (j < 16) {
      if (k > 0 && tmp[k - 1] == R[j])
        return 1;
      tmp[k] = R[j];
      j++;
      k++;
  }
  return 0;
}

static inline int uint16_sort(uint16_t *lst)
{
  int i, j, k;

  for (i = 0; i < 4; i ++){
    bitonic_merge_16(lst + 2*i*8, vld1q_u16(lst + 2*i*8), vld1q_u16(lst + (2*i+1)*8));
  }

  uint16_t tmp1[32];
  uint16_t tmp2[32];

  uint16_t * L1 = ((uint16_t *)(lst));
  uint16_t * R1 = ((uint16_t *)(lst + 16));
  uint16_t * L2 = ((uint16_t *)(lst + 32));
  uint16_t * R2 = ((uint16_t *)(lst + 48));

  if (merge(tmp1, L1, R1) == 1 || merge(tmp2, L2, R2) == 1)
    return 1;
  i = 0;
  j = 0;
  k = 0;

  while (i < 32 && j < 32) {
      if (tmp1[i] <= tmp2[j]) {
          if (k > 0 && lst[k - 1] == tmp1[i]){
            return 1;
          }
          lst[k] = tmp1[i];
          i++;
      }
      else {
          if (k > 0 && lst[k - 1] == tmp2[j]){
            return 1;
          }
          lst[k] = tmp2[j];
          j++;
      }
      k++;
  }


  while (i < 32) {
      if (k > 0 && lst[k - 1] == tmp1[i])
        return 1;
      lst[k] = tmp1[i];
      i++;
      k++;
  }


  while (j < 32) {
      if (k > 0 && lst[k - 1] == tmp2[j])
        return 1;
      lst[k] = tmp2[j];
      j++;
      k++;
  }
  return 0;

}
*/



#define uint16_MINMAX(a,b) \
do { \
  uint16_t c = b - a; \
  c >>= 15; \
  c = -c; \
  c &= a ^ b; \
  a ^= c; \
  b ^= c; \
} while(0)

static void uint16_sort(uint16_t *x,long long n)
{
  long long top,p,q,r,i;

  if (n < 2) return;
  top = 1;
  while (top < n - top) top += top;

  for (p = top;p > 0;p >>= 1) {
    for (i = 0;i < n - p;++i)
      if (!(i & p))
        uint16_MINMAX(x[i],x[i+p]);
    i = 0;
    for (q = top;q > p;q >>= 1) {
      for (;i < n - q;++i) {
        if (!(i & p)) {
          int16_t a = x[i + p];
          for (r = q;r > p;r >>= 1)
            uint16_MINMAX(a,x[i+r]);
          x[i + p] = a;
        }
      }
    }
  }
}



/*
static inline int uint16_sort(uint16_t *lst)
{
  //bitonic sort 64 elements
  //uint16_t * a = ((uint16_t *)(lst));
  //uint16_t * b = ((uint16_t *)(lst + 32));

  //initial
  int i;
  uint16x8_t a[4];
  uint16x8_t b[4];

  for (i = 0; i < 4; i++){
    //initial load
    a[i] = vld1q_u16(lst + 8*i);
    b[i] = vld1q_u16(lst + 32 + 8*i);

    //order 2
    vminmaxq(a[i], b[i]);
    vzipq(a[i], b[i]);

    //order 4
    b[i] = vrev32q_u16(b[i]);
    vminmaxq(a[i], b[i]);
    vtrn16q(a[i], b[i]);
    vminmaxq(a[i], b[i]);
    vzipq(a[i], b[i]);

    //order 8
    b[i] = vrev64q_u16(b[i]);
    vminmaxq(a[i], b[i]);
    vtrn32q(a[i], b[i]);
    vminmaxq(a[i], b[i]);
    vtrn16q(a[i], b[i]);
    vminmaxq(a[i], b[i]);
    vzipq(a[i], b[i]);

    //order16
    b[i] = vrev128q_u16(b[i]); 
    vminmaxq(a[i], b[i]);

    vtrn64q(a[i] ,b[i]);
    vminmaxq(a[i], b[i]);

    vtrn32q(a[i], b[i]);
    vminmaxq(a[i], b[i]);

    vtrn16q(a[i], b[i]);
    vminmaxq(a[i], b[i]);

    vzipq(a[i], b[i]);
  }

  for (i = 0; i < 4; i+= 2){
    //a0 b0
    //a1 b1 -> b1 a1
    a[i+1] = vrev128q_u16(a[i+1]);
    b[i+1] = vrev128q_u16(b[i+1]); 

    vminmaxq(a[i], b[i+1]);
    vminmaxq(b[i], a[i+1]);

    //a[0] b[0] -> a[0] b[1]
    //b[1] a[1] -> b[0] a[1]
    //change position
    vminmaxq(a[i], b[i]);
    vminmaxq(b[i+1], a[i+1]);

    //change by vtrn64q
    vtrn64q(a[i], b[i]);
    vtrn64q(b[i+1], a[i+1]);

    vminmaxq(a[i], b[i]);
    vminmaxq(b[i+1], a[i+1]);

    //change by vtrn32q
    vtrn32q(a[i], b[i]);
    vtrn32q(b[i+1], a[i+1]);

    vminmaxq(a[i], b[i]);
    vminmaxq(b[i+1], a[i+1]);

    //change by vtrn16q
    vtrn16q(a[i], b[i]);
    vtrn16q(b[i+1], a[i+1]);

    vminmaxq(a[i], b[i]);
    vminmaxq(b[i+1], a[i+1]);

    vzipq(a[i], b[i]);
    vzipq(b[i+1], a[i+1]);
    // a[0] b[1] b[0] a[1]  
  }

  //a0 a1 b0 b1
  //a2 a3 b2 b3

  a[2] = vrev128q_u16(a[2]);
  a[3] = vrev128q_u16(a[3]);
  b[2] = vrev128q_u16(b[2]);
  b[3] = vrev128q_u16(b[3]);

  //a2 a3 b2 b3 -> b3 b2 a3 a2
  vminmaxq(a[0], b[3]);
  vminmaxq(a[1], b[2]);
  vminmaxq(b[0], a[3]);
  vminmaxq(b[1], a[2]);

  //a0 a1 b0 b1 -> a0 a1 b3 b2 
  //b3 b2 a3 a2 -> b0 b1 a3 a2
  b[0] = vrev128q_u16(b[0]);
  b[1] = vrev128q_u16(b[1]);
  a[3] = vrev128q_u16(a[3]);
  a[2] = vrev128q_u16(a[2]);
  
  //b0 b1 a3 a2 -> b1 b0 a2 a3
  vminmaxq(a[0], b[1]);
  vminmaxq(a[1], b[0]);
  vminmaxq(b[3], a[2]);
  vminmaxq(b[2], a[3]);

  //a0 a1 b3 b2 -> a0 b1 b3 a2
  //b1 b0 a2 a3 -> a1 b0 b2 a3
  b[0] = vrev128q_u16(b[0]);
  b[2] = vrev128q_u16(b[2]);
  a[1] = vrev128q_u16(a[1]);
  a[3] = vrev128q_u16(a[3]);

  vminmaxq(a[0], a[1]);
  vminmaxq(a[2], a[3]);
  vminmaxq(b[1], b[0]);
  vminmaxq(b[3], b[2]);

  //64
  //a0 b1 b3 a2
  //a1 b0 b2 a3
  a[1] = vtrn64q(a[1]);
  b[0] = vtrn64q(b[0]);
  b[2] = vtrn64q(b[2]);
  a[3] = vtrn64q(a[3]);

  vminmaxq(a[0], a[1]);
  vminmaxq(b[1], b[0]);
  vminmaxq(b[3], b[2]);
  vminmaxq(a[2], a[3]);

  //32
  a[1] = vtrn32q(a[1]);
  b[0] = vtrn32q(b[0]);
  b[2] = vtrn32q(b[2]);
  a[3] = vtrn32q(a[3]);

  vminmaxq(a[0], a[1]);
  vminmaxq(b[1], b[0]);
  vminmaxq(b[3], b[2]);
  vminmaxq(a[2], a[3]);

  //16
  a[1] = vtrn16q(a[1]);
  b[0] = vtrn16q(b[0]);
  b[2] = vtrn16q(b[2]);
  a[3] = vtrn16q(a[3]);

  vminmaxq(a[0], a[1]);
  vminmaxq(b[1], b[0]);
  vminmaxq(b[3], b[2]);
  vminmaxq(a[2], a[3]);

  uint16x8x2_t tmp1 = vzipq_u16(a[0], a[1]);
  uint16x8x2_t tmp2 = vzipq_u16(b[1], b[0]);
  uint16x8x2_t tmp3 = vzipq_u16(b[3], b[2]);
  uint16x8x2_t tmp4 = vzipq_u16(a[2], a[3]);
  //a0 b1 b3 a2
  //a1 b0 b2 a3

  vst1q_u16(lst, tmp1.val[0]);
  vst1q_u16(lst + 8, tmp2.val[0])
  vst1q_u16(lst + 16, tmp3.val[0])
  vst1q_u16(lst + 24, tmp4.val[0])
  vst1q_u16(lst + 32, tmp1.val[1])
  vst1q_u16(lst + 40, tmp2.val[1])
  vst1q_u16(lst + 48, tmp3.val[1])
  vst1q_u16(lst + 56, tmp4.val[1])

  for (i = 1; i < 64; i++){
    if ( lst[i-1] == lst[i])
      return 1;
  }
  return 0;
}
*/

#endif

