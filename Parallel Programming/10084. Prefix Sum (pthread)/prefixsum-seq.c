#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#include <pthread.h>

#define MAXN 10000005
#define MAX_THREAD 6

uint32_t prefix_sum[MAXN];
uint32_t key;
int edge[MAX_THREAD];
int id[MAX_THREAD];

void *add(void *arg) {
  int id = *((int*)arg);
  int start = 0;
  int end = edge[id];
  if (id != 0)
    start = edge[id - 1];

  prefix_sum[start] = encrypt(start, key);
  for (int i = start + 1; i < end; ++i)
    prefix_sum[i] = prefix_sum[i - 1] + encrypt(i, key);

  pthread_exit(NULL);
}

void *final_add(void *arg) {
  int id = *((int*)arg);
  if (id == 0)
    pthread_exit(NULL);

  int start = edge[id - 1];
  int end = edge[id];

  for (int i = start; i < end - 1; ++i)
    prefix_sum[i] += prefix_sum[start - 1];

  pthread_exit(NULL);
}

int main() {
  int n;
  while (scanf("%d %" PRIu32, &n, &key) == 2) {
    pthread_t threads[MAX_THREAD];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 1; i <= MAX_THREAD; ++i) {
      if (i == MAX_THREAD)
        edge[i - 1] = n + 1;
      else
        edge[i - 1] = i * (n / MAX_THREAD);
    }

    for (int i = 0; i < MAX_THREAD; ++i) {
      id[i] = i;
      pthread_create(&threads[i], &attr, add, (void*)(id + i));
    }

    for (int i = 0; i < MAX_THREAD; ++i)
      pthread_join(threads[i], NULL);

    for (int i = 1; i < MAX_THREAD; i++)
      prefix_sum[edge[i] - 1] += prefix_sum[edge[i - 1] - 1];

    for (int i = 0; i < MAX_THREAD; ++i) {
      id[i] = i;
      pthread_create(&threads[i], &attr, final_add, (void*)(id + i));
    }

    for (int i = 0; i < MAX_THREAD; ++i)
      pthread_join(threads[i], NULL);

    output(prefix_sum, n);

    pthread_attr_destroy(&attr);
  }
  return 0;
}
