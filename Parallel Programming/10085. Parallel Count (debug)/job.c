#include "utils.h"

int ret[128];
int run(int n, int key) {
  f(n, key, ret, ret + 32, ret + 64, ret + 96);
  return ret[0] + ret[32] + ret[64] + ret[96];
}
