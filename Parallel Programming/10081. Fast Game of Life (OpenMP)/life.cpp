#include <omp.h>
#include <stdlib.h>
#include <cstdio>

const int MAXN = 2010;

bool tableA[MAXN][MAXN];
bool tableB[MAXN][MAXN];

int main() {
  int n, generation;
  scanf("%d%d", &n, &generation);

  for (int i = 1; i <= n; i++) {
    char temp[2010];
    scanf("%s", temp);
    for (int j = 1; j <= n; j++)
      tableA[i][j] = (temp[j - 1] == '1');
  }

#pragma omp parallel
  for (int g = 0; g < generation; g++) {
    int now = g % 2;
    if (now == 0) {
#pragma omp for
      for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++) {
          int nln = tableA[i + 1][j] + tableA[i - 1][j] + tableA[i][j + 1] + tableA[i][j - 1] + tableA[i + 1][j + 1] + tableA[i + 1][j - 1] + tableA[i - 1][j + 1] + tableA[i - 1][j - 1];
          if (tableA[i][j]) {
            if (nln != 2 && nln != 3)
              tableB[i][j] = false;
            else
              tableB[i][j] = true;
          } else {
            if (nln == 3)
              tableB[i][j] = true;
            else
              tableB[i][j] = false;
          }
        }
    } else {
#pragma omp for
      for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++) {
          int nln = tableB[i + 1][j] + tableB[i - 1][j] + tableB[i][j + 1] + tableB[i][j - 1] + tableB[i + 1][j + 1] + tableB[i + 1][j - 1] + tableB[i - 1][j + 1] + tableB[i - 1][j - 1];
          if (tableB[i][j]) {
            if (nln != 2 && nln != 3)
              tableA[i][j] = false;
            else
              tableA[i][j] = true;
          } else {
            if (nln == 3)
              tableA[i][j] = true;
            else
              tableA[i][j] = false;
          }
        }
    }
  }

  if (generation % 2 == 0) {
    for (int i = 1; i <= n; i++) {
      for (int j = 1; j <= n; j++)
        printf("%c", tableA[i][j] ? '1' : '0');
      printf("\n");
    }
  } else {
    for (int i = 1; i <= n; i++) {
      for (int j = 1; j <= n; j++)
        printf("%c", tableB[i][j] ? '1' : '0');
      printf("\n");
    }
  }

  return 0;
}
