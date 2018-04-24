#include <cstdio>

const int MAXN = 501;
int A[MAXN][MAXN];
int B[MAXN][MAXN];

int main() {
  int aH, aW, bH, bW;
  while(scanf("%d%d%d%d", &aH, &aW, &bH, &bW) == 4) {
    for (int i = 0; i < aH; i++)
      for (int j = 0; j < aW; j++)
        scanf("%d", &A[i][j]);
    for (int i = 0; i < bH; i++)
      for (int j = 0; j < bW; j++)
        scanf("%d", &B[i][j]);

    unsigned long long minDiff = (unsigned long long)255 * 255 * 500 * 500 + 1;
    int ansI = 0, ansJ = 0;
#pragma omp parallel for
    for (int i = 0; i <= aH - bH; i++)
      for (int j = 0; j <= aW - bW; j++) {
        unsigned long long diff = 0;
        for (int x = 0; x < bH; x++)
          for (int y = 0; y < bW; y++) {
            diff += (A[i + x][j + y] - B[x][y]) * (A[i + x][j + y] - B[x][y]);
          }

#pragma omp critical
        if (diff < minDiff) {
          minDiff = diff;
          ansI = i;
          ansJ = j;
        } else if (diff == minDiff) {
          if (ansI > i || (ansI == i && ansJ > j)) {
            ansI = i;
            ansJ = j;
          }
        }
      }

    printf("%d %d\n", ansI + 1, ansJ + 1);
  }

}
