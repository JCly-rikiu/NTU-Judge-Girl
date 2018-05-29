static inline unsigned int rotate_left(unsigned int x, unsigned int n) {
  return (x << n) | (x >> (32 - n));
}
static inline unsigned int encrypt(unsigned int m, unsigned int key) {
  return (rotate_left(m, key & 31) + key) ^ key;
}

__kernel void vecdot(const unsigned int N, const int NUMPERTHREAD, const unsigned int key1, const unsigned int key2, __global int* A, __local int* local_buffer) {
  int idx = get_global_id(0);
  int local_idx = get_local_id(0);
  int start = idx * NUMPERTHREAD;
  unsigned int sum = 0;
  for (int i = 0; i < NUMPERTHREAD; i++)
    if (start + i < N)
      sum += encrypt(start + i, key1) * encrypt(start + i, key2);

  local_buffer[local_idx] = sum;

  int top = get_local_size(0) >> 1;
  while (top > 0) {
    barrier(CLK_LOCAL_MEM_FENCE);
    if (local_idx < top)
      local_buffer[local_idx] += local_buffer[local_idx + top];
    top >>= 1;
  }

  if (local_idx == 0)
    A[get_group_id(0)] = local_buffer[0];
}
