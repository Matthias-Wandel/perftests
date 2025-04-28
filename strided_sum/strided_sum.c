//----------------------------------------------------------------------------
// Small test program exploring strided summation vs straight up summation.
// Performance substantially better for strided as it allows CPU ipipelining
// to do more in parallel before needing result of previous operation.
//
// Matthias wandel April 2025
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define NUM_FLOATS 100000

//#define USE_INTEGER 1

#ifdef USE_INTEGER
    typedef long sumtype;
    typedef int  eltype;
#else
    typedef double sumtype;
    typedef float  eltype;
#endif

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void fill_array(eltype *arr, int size)
{
    #ifdef USE_INTEGER
        // Generate ints between -3 and +3
        for (int i = 0; i < size; ++i) {
            arr[i] = (rand() / (RAND_MAX/7)) -3;
        }
    #else
        // Generate  floats between -1 and +1
        for (int i = 0; i < size; ++i) {
            arr[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        }
    #endif
}


//----------------------------------------------------------------------------
// Sum one element at a time.
//----------------------------------------------------------------------------
sumtype sum_squares_basic(const eltype *arr, int size)
{
    double total = 0.0f;
    for (int i = 0; i < size; ++i) {
        total += arr[i] * arr[i];
    }
    return total;
}
//----------------------------------------------------------------------------
// Sum oods and evenns into separate sums
//----------------------------------------------------------------------------
sumtype sum_squares_parallel_2(const eltype *arr, int size)
{
    sumtype total1 = 0.0f, total2 = 0.0f;
    for (int i = 0; i <= size; i += 2) {
        total1 += arr[i]     * arr[i];
        total2 += arr[i + 1] * arr[i + 1];
    }
    return total1 + total2;
}

//----------------------------------------------------------------------------
// Sum into four bins.
//----------------------------------------------------------------------------
sumtype sum_squares_parallel_4(const eltype *arr, int size)
{
    sumtype total1, total2, total3, total4;
    total1=total2=total3=total4=0;
    for (int i = 0; i <= size; i += 4) {
        total1 += arr[i]     * arr[i];
        total2 += arr[i + 1] * arr[i + 1];
        total3 += arr[i + 2] * arr[i + 2];
        total4 += arr[i + 3] * arr[i + 3];
    }
    return total1 + total2 + total3 + total4;
}

eltype array[NUM_FLOATS];// = (float *)malloc(NUM_FLOATS * sizeof(float));
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int main() {
    LARGE_INTEGER freq_t, start_t, end_t;

    //srand((unsigned int)time(NULL));
    fill_array(array, NUM_FLOATS);

    QueryPerformanceFrequency(&freq_t);


    // Time basic version
    QueryPerformanceCounter(&start_t);
    double result1 = sum_squares_basic(array, NUM_FLOATS);
    QueryPerformanceCounter(&end_t);
    double duration_usec1 = (double)(end_t.QuadPart - start_t.QuadPart) * 1000000.0 / freq_t.QuadPart;
    

    // Time stride 2 version
    QueryPerformanceCounter(&start_t);
    double result2 = sum_squares_parallel_2(array, NUM_FLOATS);
    QueryPerformanceCounter(&end_t);
    double duration_usec2 = (double)(end_t.QuadPart - start_t.QuadPart) * 1000000.0 / freq_t.QuadPart;

    // Time stride 4 version
    QueryPerformanceCounter(&start_t);
    double result4 = sum_squares_parallel_4(array, NUM_FLOATS);
    QueryPerformanceCounter(&end_t);
    double duration_usec4 = (double)(end_t.QuadPart - start_t.QuadPart) * 1000000.0 / freq_t.QuadPart;


    printf("Normal    sum=%7.2f, time=%5.2f usec\n",result1, duration_usec1);
    printf("Strided 2 sum=%7.2f, time=%5.2f usec\n",result2, duration_usec2);
    printf("Strided 4 sum=%7.2f, time=%5.2f usec\n",result4, duration_usec4);

    return 0;
}
