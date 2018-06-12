#include "labeling.h"

const int threads = 256;

__device__ __host__ int CeilDiv(int a, int b) { return (a - 1) / b + 1; }

__global__ void add_aux(int *pos, int text_size, int *d_aux) {
  int gindex = blockIdx.x * blockDim.x + threadIdx.x;
  int lindex = threadIdx.x;

  if (blockIdx.x != 0 && gindex < text_size)
    if (lindex == pos[gindex] - 1)
      pos[gindex] += d_aux[blockIdx.x - 1];
}

__global__ void intra_block_scan(const char *text, int *pos, int text_size, int *d_aux) {
  __shared__ int sdata[threads * 2];
  __shared__ bool fdata[threads * 2];
  __shared__ bool ofdata[threads * 2];
  __shared__ bool add1[threads * 2];

  int gindex = (blockIdx.x * blockDim.x + threadIdx.x) * 2;
  int lindex = threadIdx.x * 2;
  int tid = threadIdx.x;

  if (gindex < text_size)
    sdata[lindex] = add1[lindex] = (text[gindex] == ' ' ? 0 : 1);
  if (gindex + 1 < text_size)
    sdata[lindex + 1] = add1[lindex + 1] = (text[gindex + 1] == ' ' ? 0 : 1);
  __syncthreads();

  if (lindex == 0)
    fdata[lindex] = ofdata[lindex] = true;
  else
    fdata[lindex] = ofdata[lindex] = (sdata[lindex - 1] != sdata[lindex]);
  fdata[lindex + 1] = ofdata[lindex + 1] = (sdata[lindex] != sdata[lindex + 1]);

  int offset = 1;
  for (int d = threads; d > 0; d >>= 1) {
    __syncthreads();
    if (tid < d) {
      int ai = offset * (lindex + 1) - 1;
      int bi = offset * (lindex + 2) - 1;
      if (!fdata[bi])
        sdata[bi] += sdata[ai];
      fdata[bi] = fdata[ai] || fdata[bi];
    }
    offset <<= 1;
  }

  if (tid == 0)
    sdata[threads * 2 - 1] = 0;

  for (int d = 1; d <= threads; d <<= 1) {
    __syncthreads();
    offset >>= 1;
    if (tid < d) {
      int ai = offset * (lindex + 1) - 1;
      int bi = offset * (lindex + 2) - 1;
      int t = sdata[ai];
      sdata[ai] = sdata[bi];
      if (ofdata[ai + 1])
        sdata[bi] = 0;
      else if (fdata[ai])
        sdata[bi] = t;
      else
        sdata[bi] += t;
      fdata[ai] = false;
    }
  }
  __syncthreads();

  if (gindex < text_size)
    pos[gindex] = sdata[lindex] + add1[lindex];
  if (gindex + 1 < text_size)
    pos[gindex + 1] = sdata[lindex + 1] + add1[lindex + 1];

  if (tid == 0)
    d_aux[blockIdx.x] = sdata[threads * 2 - 1] + add1[threads * 2 - 1];
}

void labeling(const char *text, int *pos, int text_size) {
  int *d_aux;
  cudaMalloc(&d_aux, CeilDiv(text_size, threads) * sizeof(int));
  intra_block_scan<<<CeilDiv(text_size, threads * 2), threads>>>(
      text, pos, text_size, d_aux);
  add_aux<<<CeilDiv(text_size, threads * 2), threads * 2>>>(pos, text_size,
                                                            d_aux);
  cudaFree(d_aux);
}
