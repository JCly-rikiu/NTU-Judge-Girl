#include <cstdio>
#include <cstdlib>

char table[17][17];

int n;

bool ok(int position[], int next, int test) {
  if (table[next][test] == '*')
    return false;
  for (int i = 0; i < next; i++)
    if (position[i] == test || (abs(test - position[i]) == next - i))
      return false;
  return true;
}

int queen(int position[], int next) {
  if (next >= n)
    return 1;
  int sum = 0;
  for (int test = 0; test < n; test++)
    if (ok(position, next, test)) {
      position[next] = test;
      sum += queen(position, next + 1);
    }
  return sum;
}

int queen_entry(int position[], int i, int j, int k) {
  if (ok(position, 0, i))
    position[0] = i;
  else
    return 0;
  if (ok(position, 1, j))
    position[1] = j;
  else
    return 0;
  if (ok(position, 2, k))
    position[2] = k;
  else
    return 0;

  return queen(position, 3);
}

int main() {
  int it = 1;
  while(scanf("%d", &n) == 1) {
    for (int i = 0; i != n; i++)
      scanf("%s", table[i]);

    int numSolution = 0;
    int position[17] = { 0 };
#pragma omp parallel for firstprivate (position) reduction(+ : numSolution) collapse(3) schedule(dynamic)
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
          numSolution += queen_entry(position, i, j, k);
        }
      }
    }
    printf("Case %d: %d\n", it++, numSolution);
  }

  return 0;
}
