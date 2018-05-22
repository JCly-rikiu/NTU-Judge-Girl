static inline unsigned int rotate_left(unsigned int x, unsigned int n) {
  return (x << n) | (x >> (32 - n));
}
static inline unsigned int encrypt(unsigned int m, unsigned int key) {
  return (rotate_left(m, key & 31) + key) ^ key;
}

__kernel void vecdot(const unsigned int N, const int NUMPERBLOCK, const unsigned int key1, const unsigned int key2, __global int* A) {
  int idx = get_global_id(0);
  int start = idx * NUMPERBLOCK;
  unsigned int sum = 0;
  for (int i = 0; i < NUMPERBLOCK; i++)
    if (start + i < N)
      sum += encrypt(start + i, key1) * encrypt(start + i, key2);
  A[idx] = sum;
}
