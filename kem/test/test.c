

#include <stdint.h>
#include <arm_neon.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hal.h"

#define GFBITS 12
#define ITERATIONS 1000

typedef uint64x2_t vec128;

uint64_t t0, t1;
uint64_t times[ITERATIONS];

static int cmp_uint64(const void *a, const void *b){
    return (int)((*((const uint64_t*)a)) - (*((const uint64_t*)b)));
}

void vec128_mul_ref(vec128 *h, vec128 *f, vec128 *g);
void vec128_mul_test(vec128 *h, vec128 *f, vec128 *g);
void vec128_mul_new(vec128 *h, vec128 *f, vec128 *g);

int main(void){

    vec128 f[GFBITS], g[GFBITS];
    vec128 h[GFBITS], t[GFBITS];

    for(size_t i = 0; i < GFBITS; i++){
        f[i][0] = rand();
	f[i][1] = rand();
        g[i][0] = rand();
	g[i][1] = rand();
    }

    vec128_mul_ref(h, f, g);
   
    for(size_t i = 0; i < GFBITS; i++){
        t[i][0] = rand();
	t[i][1] = rand();
    } 
    vec128_mul_test(t, f, g);
    for(size_t i = 0; i < GFBITS; i++){
        if((uint64_t)h[i][0] != (uint64_t)t[i][0]){
            printf("%4zu: %16lx, %16lx, %16lx, %16lx\n", i, 
	        (uint64_t)h[i][0], (uint64_t)h[i][1],
		(uint64_t)t[i][0], (uint64_t)t[i][1]);
	}
    }
    printf("test finished!\n");
    for(size_t i = 0; i < GFBITS; i++){
        t[i][0] = rand();
	t[i][1] = rand();
    } 
    vec128_mul_new(t, f, g);
    for(size_t i = 0; i < GFBITS; i++){
        if((uint64_t)h[i][0] != (uint64_t)t[i][0]){
	    printf("%4zu: %16lx, %16lx, %16lx, %16lx\n", i, 
	        (uint64_t)h[i][0], (uint64_t)h[i][1],
                (uint64_t)t[i][0], (uint64_t)t[i][1]);
        }
    }
    printf("new finished!\n");



    for(size_t i = 0; i < ITERATIONS; i++){
        t0 = hal_get_time();
	vec128_mul_ref(h, f, g);
	t1 = hal_get_time();
	times[i] = t1 - t0;
    }
    qsort(times, ITERATIONS, sizeof(uint64_t), cmp_uint64);
    printf("ref: %ld\n", times[ITERATIONS >> 1]);


    for(size_t i = 0; i < ITERATIONS; i++){
        t0 = hal_get_time();
	vec128_mul_test(h, f, g);
	t1 = hal_get_time();
	times[i] = t1 - t0;
    }
    qsort(times, ITERATIONS, sizeof(uint64_t), cmp_uint64);
    printf("test: %ld\n", times[ITERATIONS >> 1]);

    for(size_t i = 0; i < ITERATIONS; i++){
        t0 = hal_get_time();
	vec128_mul_new(h, f, g);
	t1 = hal_get_time();
	times[i] = t1 - t0;
    }
    qsort(times, ITERATIONS, sizeof(uint64_t), cmp_uint64);
    printf("new: %ld\n", times[ITERATIONS >> 1]);



}

void vec128_mul_ref(vec128 *h, vec128 *f, vec128 *g)
{
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
}


void vec128_mul_test(uint64x2_t *h, uint64x2_t *f, uint64x2_t *g)
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


void vec128_mul_new(uint64x2_t *h, uint64x2_t *f, uint64x2_t *g)
{

	uint64x2_t buf[GFBITS];

// ========

// ========

	// 11
	buf[10] = vandq_u64(f[11], g[11]);
        buf[1] = buf[10];

	// 10-11
	buf[9] = veorq_u64(vandq_u64(f[10], g[11]), 
			vandq_u64(f[11], g[10])
			);
	buf[0] = buf[9];
	
	// 9-11
	buf[8] = veorq_u64(vandq_u64(f[10], g[10]),
			veorq_u64(vandq_u64(f[11], g[9]),
				vandq_u64(f[9], g[11])
				)
			);

        buf[7] = veorq_u64(vandq_u64(f[10], g[9]),
			vandq_u64(f[9], g[10])
			);

	// 8-11
	buf[6] = veorq_u64(vandq_u64(f[9], g[9]),
                        veorq_u64(vandq_u64(f[10], g[8]),
				vandq_u64(f[8], g[10]))
			);

        buf[5] = veorq_u64(vandq_u64(f[9], g[8]),
			vandq_u64(f[8], g[9])
			);

	buf[7] = veorq_u64(buf[7],
			veorq_u64(vandq_u64(f[11], g[8]),
				vandq_u64(f[8], g[11])
				)
			);

	// 7-11
	buf[6] = veorq_u64(buf[6],
			veorq_u64(vandq_u64(f[11], g[7]),
				vandq_u64(f[7], g[11])
				)
			);
	
        
	buf[5] = veorq_u64(buf[5],
			veorq_u64(vandq_u64(f[10], g[7]),
				vandq_u64(f[7], g[10])
				)
			);
	
	buf[4] = veorq_u64(vandq_u64(f[8], g[8]),
                        veorq_u64(vandq_u64(f[9], g[7]),
				vandq_u64(f[7], g[9]))
			);

        buf[3] = veorq_u64(vandq_u64(f[8], g[7]),
			vandq_u64(f[7], g[8])
			);

	// 6-11
	buf[5] = veorq_u64(buf[5],
			veorq_u64(vandq_u64(f[11], g[6]),
				vandq_u64(f[6], g[11])
				)
			);
	
	buf[4] = veorq_u64(buf[4],
			veorq_u64(vandq_u64(f[10], g[6]),
				vandq_u64(f[6], g[10])
				)
			);
    
	buf[3] = veorq_u64(buf[3],
			veorq_u64(vandq_u64(f[9], g[6]),
				vandq_u64(f[6], g[9])
				)
			);

	buf[2] = veorq_u64(vandq_u64(f[7], g[7]),
                        veorq_u64(vandq_u64(f[8], g[6]),
				vandq_u64(f[6], g[8]))
			);

	buf[1] = veorq_u64(buf[1],
			veorq_u64(vandq_u64(f[7], g[6]), 
				vandq_u64(f[6], g[7]))
			);


	// 5-9
	buf[10] = veorq_u64(buf[10], vandq_u64(f[5], g[5]));
	
	buf[2] = veorq_u64(buf[2],
			veorq_u64(vandq_u64(f[9], g[5]),
				vandq_u64(f[5], g[9])
				)
			);

	buf[1] = veorq_u64(buf[1],
			veorq_u64(vandq_u64(f[8], g[5]),
				vandq_u64(f[5], g[8])
				)
			);

	// 4-9
	buf[10] = veorq_u64(buf[10],
			veorq_u64(vandq_u64(f[6], g[4]),
				vandq_u64(f[4], g[6])
				)
			);

	buf[1] = veorq_u64(buf[1],
			veorq_u64(vandq_u64(f[9], g[4]),
				vandq_u64(f[4], g[9])
				)
			);

	// 3-7
	buf[10] = veorq_u64(buf[10],
			veorq_u64(vandq_u64(f[7], g[3]),
				vandq_u64(f[3], g[7])
				)
			);

        // 2-7

	// 1-6

	buf[4] = veorq_u64(buf[4],
			veorq_u64(vandq_u64(f[11], g[5]),
				vandq_u64(f[5], g[11])
				)
			);

	buf[3] = veorq_u64(buf[3],
			veorq_u64(vandq_u64(f[10], g[5]),
				vandq_u64(f[5], g[10])
				)
			);

	buf[3] = veorq_u64(buf[3],
			veorq_u64(vandq_u64(f[11], g[4]),
				vandq_u64(f[4], g[11])
				)
			);

	buf[2] = veorq_u64(buf[2],
			veorq_u64(vandq_u64(f[10], g[4]),
				vandq_u64(f[4], g[10])
				)
			);
      
	buf[2] = veorq_u64(buf[2],
			veorq_u64(vandq_u64(f[11], g[3]),
				vandq_u64(f[3], g[11])
				)
			);


	buf[1] = veorq_u64(buf[1],	
		    veorq_u64(veorq_u64(vandq_u64(f[11], g[2]), vandq_u64(f[10], g[3])),
				veorq_u64(vandq_u64(f[3], g[10]), vandq_u64(f[2], g[11]))
					)
			);

	buf[0] = veorq_u64(buf[0],
			veorq_u64(vandq_u64(f[1], g[11]), veorq_u64(veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[11], g[1]), vandq_u64(f[10], g[2])),
							veorq_u64(vandq_u64(f[9], g[3]), vandq_u64(f[8], g[4]))), 
						veorq_u64(veorq_u64(vandq_u64(f[7], g[5]), vandq_u64(f[6], g[6])), veorq_u64(vandq_u64(f[5], g[7]), vandq_u64(f[4], g[8])
								)
							)
						), 
					veorq_u64(vandq_u64(f[3], g[9]), vandq_u64(f[2], g[10])
						)
					)
				)
			);



	buf[10] = veorq_u64(buf[10],
			veorq_u64(vandq_u64(f[8], g[2]),
				vandq_u64(f[2], g[8])
				)
			);

	buf[10] = veorq_u64(buf[10],
			veorq_u64(vandq_u64(f[9], g[1]),
				vandq_u64(f[1], g[9])
				)
			);

	buf[10] = veorq_u64(buf[10],
			veorq_u64(vandq_u64(f[10], g[0]),
				vandq_u64(f[0], g[10])
				)
			);


	buf[9] = veorq_u64(buf[9],
			veorq_u64(veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[9], g[0]), vandq_u64(f[0], g[9])),
						veorq_u64(vandq_u64(f[8], g[1]), vandq_u64(f[1], g[8]))), 
					veorq_u64(veorq_u64(vandq_u64(f[7], g[2]), vandq_u64(f[2], g[7])), veorq_u64(vandq_u64(f[3], g[6]), vandq_u64(f[6], g[3])))), 
				veorq_u64(vandq_u64(f[4], g[5]), vandq_u64(f[5], g[4])
					)
				)
			);

	h[11] = veorq_u64(buf[8],
			veorq_u64(veorq_u64(vandq_u64(f[6], g[5]), vandq_u64(f[5], g[6])), veorq_u64(veorq_u64(vandq_u64(f[11], g[0]), vandq_u64(f[0], g[11])), 
					veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[10], g[1]), vandq_u64(f[1], g[10])), 
							veorq_u64(vandq_u64(f[9], g[2]), vandq_u64(f[2], g[9]))), 
						veorq_u64(veorq_u64(vandq_u64(f[3], g[8]), vandq_u64(f[8], g[3])), 
							veorq_u64(vandq_u64(f[7], g[4]), vandq_u64(f[4], g[7])
								)
							)
						)
					)
				)
			);

        h[10] = veorq_u64(buf[10], buf[7]);
        h[9] = veorq_u64(buf[9], buf[6]);

	buf[7] = veorq_u64(buf[7],
			veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[0], g[7]), vandq_u64(f[7], g[0])),
					veorq_u64(vandq_u64(f[6], g[1]), vandq_u64(f[1], g[6]))),
				veorq_u64(veorq_u64(vandq_u64(f[5], g[2]), vandq_u64(f[2], g[5])),
					veorq_u64(vandq_u64(f[4], g[3]), vandq_u64(f[3], g[4])
						)
					)
				)
			);

	buf[6] = veorq_u64(buf[6],
			veorq_u64(veorq_u64(vandq_u64(f[3], g[3]), veorq_u64(vandq_u64(f[6], g[0]), vandq_u64(f[0], g[6]))),
				veorq_u64(veorq_u64(vandq_u64(f[5], g[1]), vandq_u64(f[1], g[5])), veorq_u64(vandq_u64(f[4], g[2]), vandq_u64(f[2], g[4])
						)
					)
				)
			);

	buf[8] = veorq_u64(buf[8],
			veorq_u64(vandq_u64(f[4], g[4]), veorq_u64(veorq_u64(veorq_u64(vandq_u64(f[0], g[8]), vandq_u64(f[8], g[0])),
						veorq_u64(vandq_u64(f[7], g[1]), vandq_u64(f[1], g[7]))),
					veorq_u64(veorq_u64(vandq_u64(f[6], g[2]), vandq_u64(f[2], g[6])),
						veorq_u64(vandq_u64(f[5], g[3]), vandq_u64(f[3], g[5])
							)
						)
					)
				)
			);

	h[7] = veorq_u64(buf[7], buf[4]);
	h[6] = veorq_u64(buf[6], buf[3]);
	h[8] = veorq_u64(buf[8], buf[5]);

	buf[4] = veorq_u64(buf[4], veorq_u64(vandq_u64(f[2], g[2]), veorq_u64(veorq_u64(vandq_u64(f[4], g[0]), vandq_u64(f[0], g[4])), veorq_u64(vandq_u64(f[3], g[1]), vandq_u64(f[1], g[3])))));
	buf[3] = veorq_u64(buf[3], veorq_u64(veorq_u64(vandq_u64(f[2], g[1]),vandq_u64(f[1], g[2])), veorq_u64(vandq_u64(f[0], g[3]), vandq_u64(f[3], g[0]))));
	buf[5] = veorq_u64(buf[5],
			veorq_u64(veorq_u64(vandq_u64(f[0], g[5]), vandq_u64(f[5], g[0])), veorq_u64(veorq_u64(vandq_u64(f[4], g[1]), vandq_u64(f[1], g[4])), veorq_u64(vandq_u64(f[3], g[2]), vandq_u64(f[2], g[3])))));

	h[4] = veorq_u64(buf[4], buf[1]);
        h[3] = veorq_u64(buf[3], buf[0]);
	h[5] = veorq_u64(buf[5], buf[2]);

	h[1] = veorq_u64(buf[1], veorq_u64(vandq_u64(f[0], g[1]), vandq_u64(f[1], g[0])));
	h[0] = veorq_u64(buf[0], vandq_u64(f[0], g[0]));
	h[2] = veorq_u64(buf[2], veorq_u64(vandq_u64(f[1], g[1]), veorq_u64(vandq_u64(f[0], g[2]), vandq_u64(f[2], g[0]))));

// ========



// ========


        



// ========

// ========

// ========


// ========


}
