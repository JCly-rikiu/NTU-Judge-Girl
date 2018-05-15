#include <cstdio>

const int MAXN = 2048;
const long long INF = (1LL << 60);
int N;
long long dp[MAXN * MAXN], SZ[MAXN + 1];

int main() {
  while (scanf("%d", &N) == 1) {
    for (int i = 0; i <= N; i++)
      scanf("%lld", SZ + i);
    for (int i = 0; i < N; i++)
      dp[i * N + i] = 0;

#pragma omp parallel
    for (int i = 1; i < N; i++) {
#pragma omp for
      for (int j = 0; j < N - i; j++) {
        int l = j, r = j + i;
        int sl = l * N, sr = r * N;
        long long local = INF;
        long long partial = SZ[l] * SZ[r + 1];
        for (int k = l; k < r; k++) {
          long long t = dp[sl + k] + dp[sr + (k + 1)] + SZ[k + 1] * partial;
          if (t < local)
            local = t;
        }
        dp[sl + r] = local;
        dp[sr + l] = local;
      }
    }

    printf("%lld\n", dp[0 * N + N - 1]);
  }
  return 0;
}
