//A fun experiment about performance on x86_64:
//It is organized into different thread function for ease of decompilation:
//Godbolt and gcc effectively render each one of them as a separate unit so
//that we can check how long each function takes separately
//It was inspired by:
// https://www.youtube.com/watch?v=RrHGX1wwSYM
//Often x86 processor tend to have fewer ports for integer division "idiv()"
//than they do for float or double. On top of that the algorithm used can
//differ, as often lookup tables are implemented for some types of division but
//not all of them. 
//
//Division is by nature pretty slow and often cannot be fully parallelized, not
//without using SIMD or similar parallelization instructions, which are only
//appliable in some cases. 
//
//The results will differ, unfortunately, between compilers, chip maker 
//(AMD vs Intel) or even between different generations of processors that use
//different architectures (Zen4 vs Zen5).
//Some guidelines for gcc are the following.
//gcc -O0 -fno-strict-aliasing -fwrapv -fno-strict-overflow -lm


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>


#define MAXCYCLE 1000


void *integer_divide(void *);
void *doubleCast_divide(void *);
void *double_divide(void*);
void *float_divide(void *);


void *integer_divide(void *) {
    srand(time(NULL));  
    int random=rand() %1000;
    unsigned long long msr;
    asm volatile ("rdtsc\n\t"
        "shl $32, %%rdx\n\t"
        "or %%rdx, %0"
        :"=a" (msr)
        :
        :"rdx");
    long long *pointer = &msr;

    int tscounter = (int) ((*pointer)>>4);

    int result = tscounter % random;
    //void *result = &result;
    return NULL;
}


void *doubleCast_divide(void *) {
    srand(time(NULL));  
    int random=rand() %1000;
    unsigned long long msr;
    asm volatile ("rdtsc\n\t"
        "shl $32, %%rdx\n\t"
        "or %%rdx, %0"
        :"=a" (msr)
        :
        :"rdx");
    long long *pointer = &msr;

    int tscounter = (int) ((*pointer)>>4);
    double quotient = (double) tscounter / (double) random;
    quotient = trunc(quotient);
    double remainder = (double) tscounter - (double) random * quotient;
  
    int result = (int) remainder;
    //void *result = &result;
    return NULL;
}

void *double_divide(void*){
   srand(time(NULL));  
    int r=rand() %1000;
    int ra = r & 0x3FF;
    double nd= (double) (r >> 4);
    double rand = ra+nd;

    unsigned long long msr;
    asm volatile ("rdtsc\n\t"
        "shl $32, %%rdx\n\t"
        "or %%rdx, %0"
        :"=a" (msr)
        :
        :"rdx");
    long long *pointer = &msr;

    double tscounter = (double) ((*pointer)>>4);
    
    double quotient = tscounter / rand;
    double remainder = tscounter - (rand * quotient);

    //void *result = &remainder;
    return NULL;


}


void *float_divide(void *) {
   srand(time(NULL));  
    int r=rand() %1000;
    int ra = r & 0x3FF;
    float nd= (float) (r >> 4);
    float rand = ra+nd;

    unsigned long long msr;
    asm volatile ("rdtsc\n\t"
        "shl $32, %%rdx\n\t"
        "or %%rdx, %0"
        :"=a" (msr)
        :
        :"rdx");
    long long *pointer = &msr;
    


    float ts = (float) (msr & 0x3FF);
    float ter = (float) ((*pointer)>>4);
    float tscounter = ts+ter;

    float quotient = tscounter / rand;
    float remainder = tscounter - (rand * quotient);

    //void *result = &remainder;
    return NULL;



}




int main(void) {
  pthread_t thread0, thread1, thread2, thread3;

  int retv0, retv1, retv2, retv3;
   
  if (retv0=pthread_create(&thread0, NULL, integer_divide, NULL) !=0) {puts("Thread 0 failed");}
  if (retv1=pthread_create(&thread1, NULL, doubleCast_divide, NULL) !=0) {puts("Thread 1 failed");}
  if (retv2=pthread_create(&thread2, NULL, double_divide, NULL) !=0) {puts("Thread 2 failed");}
  if (retv3=pthread_create(&thread3, NULL, float_divide, NULL) !=0) {puts("Thread 3 failed");}
       
  pthread_join(thread0, NULL);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);    
    return 0;
}




