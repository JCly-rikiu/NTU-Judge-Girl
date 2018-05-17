#include <cstdio>
#include <cstdlib>
#include <omp.h>

const int MAXN = 2010;

bool table[2][MAXN][MAXN];

int main() {
  int n, generation;
  scanf("%d%d", &n, &generation);

  for (int i = 1; i <= n; i++) {
    char temp[2010];
    scanf("%s", temp);
    for (int j = 1; j <= n; j++)
      table[0][i][j] = (temp[j - 1] == '1');
  }

#pragma omp parallel
  for (int g = 0, now = 0; g < generation; g++, now = 1 - now) {
#pragma omp for
    for (int i = 1; i <= n; i++)
      for (int j = 1; j <= n; j++) {
        int nln = table[now][i + 1][j] + table[now][i - 1][j] + table[now][i][j + 1] + table[now][i][j - 1] + table[now][i + 1][j + 1] + table[now][i + 1][j - 1] + table[now][i - 1][j + 1] + table[now][i - 1][j - 1];
        if (table[now][i][j]) {
          if (nln != 2 && nln != 3)
            table[1 - now][i][j] = false;
          else
            table[1 - now][i][j] = true;
        } else {
          if (nln == 3)
            table[1 - now][i][j] = true;
          else
            table[1 - now][i][j] = false;
        }
      }
  }

  int now = generation % 2;
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= n; j++)
      printf("%c", table[now][i][j] ? '1' : '0');
    printf("\n");
  }

  return 0;
}
