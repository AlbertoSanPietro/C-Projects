#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define MAXL 4096
#define FAILURE 1

//An implementation of the dual pivot quicksort by Vladimir Yaroslavskiy 
//with insertion sort for arrays of a smaller size (less than 32)
//I added some basic overflow checks
//Since MAXINT and similar are not used the code has to be compiled with the
//following gcc flags:
//// -fno-strict-aliasing 
///Unnecessary, as there is no undefined behaviour as of
// yet (especially no illegal lvalue statements according to the ANSI C
// Standard)
// -fwrapv 
// Again, more of a nice-to-have in this situation as there are no
// bitwise operations nor situation in which bits can be moved out of their
// structure
// -fno-strict-overflow 
// This is often the required flag for most level of optimization, as it allows
// for overflow checks, which are UB in ANSI C
// gcc -O3 is infamous for optimizing away checks made after the overflow
// The checks here are unaffected by these flags, as OF cannot happen here

void dualPivotQS(int[], int, int);
void swap(int[], int, int);
void print(int[], int);
void insSort(int[], int, int);
void qsort(int[], int, int);

int main(void) 
{
    int vett[MAXL];
    double time1, timedif;

    unsigned int n = 0;
        printf("Insert the size of the array\n");
    if (scanf("%u", &n) != 1) {  
        puts("EINVAL");
        return FAILURE;
    }

    if (n == 0 || n > MAXL) {  // Bounds check
        puts("EINVAL");
        return FAILURE;
    }

    printf("\nInsert the elements of the array\n");
    for (unsigned int i = 0; i < n; i++) {
        if (scanf("%d", &vett[i]) != 1) { 
            puts("Invalid element");
            return FAILURE;
        }
    }

    time1 = ((double) clock()) / CLOCKS_PER_SEC;
    insSort(vett, 0, n-1);
    timedif = (((double) clock()) / CLOCKS_PER_SEC) - time1;
    printf("The elapsed time is %lf seconds\n", timedif);
    print(vett, n);

    return 0;
}

void swap(int v[], int i, int j)
{
    int temp = v[i];
    v[i] = v[j];
    v[j] = temp;
}

void print(int v[], int n) 
{
    printf("\n");
    for (int i = 0; i < n; i++)
        printf("%d\t", v[i]);
    printf("\n");
}


void dualPivotQS(int v[], int sx, int dx)
{
    if (sx >= dx)
        return;

    // Choosing pivots
    if (v[sx] > v[dx])
        swap(v, sx, dx);

    int pivot1 = v[sx];
    int pivot2 = v[dx];
    
    int less = sx + 1;
    int great = dx - 1;

    for (int k = less; k <= great; k++)
    {
        if (v[k] < pivot1)
        {
            swap(v, k, less);
            less++;
        }
        else if (v[k] > pivot2)
        {
            while (k < great && v[great] > pivot2)
                great--;
            swap(v, k, great);
            great--;
            if (v[k] < pivot1)
            {
                swap(v, k, less);
                less++;
            }
        }
    }
    
    less--;
    great++;

    swap(v, sx, less);
    swap(v, dx, great);

    // Recursively sort partitions
    dualPivotQS(v, sx, less - 1);
    dualPivotQS(v, less + 1, great - 1);
    dualPivotQS(v, great + 1, dx);
}

void qsort(int v[], int left, int right)
{
    int i = 0, last;
    
    if (left >= right)
        return; 
    swap(v, left, (left+right)/2); 
    last=left;
    for (i = left+1; i<=right; i++){
        if(v[i] < v[left])
            swap(v, ++last, i); 
    }   
    swap (v, left, last);   
    qsort(v, left, last-1);
    qsort (v, last+1, right);
}



void insSort(int v[], int sx, int dx)
{
    int n = dx - sx + 1;
    if (n < 16)
    {
        for (int i = sx; i <= dx; i++)
        {
            for (int j = i + 1; j <= dx; j++)
            {
                if (v[i] > v[j])
                {
                    swap(v, i, j);
                }
            }
        }
    } else if(n < 32) {
        qsort(v, sx, dx);
    }
    else
    {
        dualPivotQS(v, sx, dx);
    }
}
