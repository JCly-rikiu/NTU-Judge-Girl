#include <cstdio>

int w[10010];
int v[10010];

int dp[2][1000010];

int main() {
  int n, m;
  scanf("%d%d", &n, &m);

  for (int i = 0; i < n; i++)
    scanf("%d%d", w + i, v + i);

  for (int i = 0; i < n; i++) {
    int now = i % 2;
    int last = 1 - now;

#pragma omp parallel for
    for (int j = 0; j <= m; j++) {
      if (j < w[i]) {
        dp[now][j] = dp[last][j];
        continue;
      }

      if (dp[last][j - w[i]] + v[i] > dp[last][j])
        dp[now][j] = dp[last][j - w[i]] + v[i];
      else
        dp[now][j] = dp[last][j];
    }
  }

  printf("%d\n", dp[(n - 1) % 2][m]);

  return 0;
}
