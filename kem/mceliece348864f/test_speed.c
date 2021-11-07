#include <stdio.h>
#include <stdint.h>

static inline unsigned int get_cyclecount (void)
{
 unsigned int value;
 // Read CCNT Register
 asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value));  
 return value;
}

static inline void init_perfcounters (int32_t do_reset, int32_t enable_divider)
{
 // in general enable all counters (including cycle counter)
 int32_t value = 1;

 // peform reset:  
 if (do_reset)
 {
   value |= 2;     // reset all counters to zero.
   value |= 4;     // reset cycle counter to zero.
 } 

 if (enable_divider)
   value |= 8;     // enable "by 64" divider for CCNT.

 value |= 16;

 // program the performance-counter control-register:
 asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));  

 // enable all counters:  
 asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));  

 // clear overflows:
 asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
}

int main()
{
 // init counters:
 init_perfcounters (1, 0); 

 // measure the counting overhead:
 unsigned int overhead = get_cyclecount();
 overhead = get_cyclecount() - overhead;    


 // do some stuff here..
 uint64_t a = 0;
 uint64_t b = 100;
 uint64_t c = 0;

 uint32_t t0, t1, t2;
 uint32_t sum = 0;

for (int i = 0; i < 20; i++){
   t0 = get_cyclecount();

   for (int j = 0; j < 1000; j++){
    c = a ^ b;
    }
    
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;
    c = a ^ b;

    t1 = get_cyclecount();
    printf (" %d \n", t1-t0);

}

printf ("+++++++++++++++++++++++++++++++++++=\n");


for (int i = 0; i < 20; i++){
   t0 = get_cyclecount();

   for (int j = 0; j < 100000; j++){
    c = a ^ b;
    }

    t1 = get_cyclecount();
    printf (" %d \n", t1-t0);

}
//    printf ("function took exactly %d cycles (%d overhead)\n", t1-t0, overhead);

 return 0;
}















