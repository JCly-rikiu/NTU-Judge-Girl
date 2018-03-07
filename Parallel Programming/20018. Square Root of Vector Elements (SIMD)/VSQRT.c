#include "VSQRT.h"
 
#include <math.h>
#include <x86intrin.h>
 
void sqrt2(float *begin, float *end) {
  for (; begin < end; begin += 8)
    _mm256_storeu_ps(begin, _mm256_sqrt_ps(_mm256_loadu_ps(begin)));
}

