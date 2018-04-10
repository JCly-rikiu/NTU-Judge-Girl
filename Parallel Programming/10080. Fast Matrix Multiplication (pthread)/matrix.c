#include "matrix.h"

#include <pthread.h>
#define MAX_THREAD 6

int id[MAX_THREAD];
int edge[MAX_THREAD];
unsigned long gA[2048][2048], gB[2048][2048], gC[2048][2048];
int gN;

void *process(void *arg) {
  int id = *((int*)arg);

  int start = 0;
  if (id != 0)
    start = edge[id - 1];
  int end = edge[id];

  for (int i = start; i < end; ++i) {
    for (int j = 0; j < gN; ++j) {
      unsigned long sum = 0;  // overflow, let it go.
      for (int k = 0; k < gN; k++)
        sum += gA[i][k] * gB[j][k];
      gC[i][j] = sum;
    }
  }

  pthread_exit(NULL);
}

void transpose(int N, unsigned long B[][2048]) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < i; ++j) {
      int temp = B[i][j];
      B[i][j] = B[j][i];
      B[j][i] = temp;
    }
  }
}

void multiply(int N, unsigned long A[][2048], unsigned long B[][2048], unsigned long C[][2048]) {
  transpose(N, B);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      gA[i][j] = A[i][j];
      gB[i][j] = B[i][j];
    }
  gN = N;

  for (int i = 0; i < MAX_THREAD; ++i) {
    if (i == MAX_THREAD - 1)
      edge[i] = N + 1;
    else
      edge[i] = (i + 1) * (N / MAX_THREAD);
  }

  pthread_t threads[MAX_THREAD];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for (int i = 0; i < MAX_THREAD; ++i) {
    id[i] = i;
    pthread_create(&threads[i], &attr, process, (void*)(id + i));
  }

  for (int i = 0; i < MAX_THREAD; ++i)
    pthread_join(threads[i], NULL);

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      C[i][j] = gC[i][j];

  pthread_attr_destroy(&attr);
}
