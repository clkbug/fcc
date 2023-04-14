void putint(int x) {
  if (x < 0) {
    putchar('-');
    x = -x;
  }
  if (x >= 10) {
    putint(x / 10);
  }
  putchar(x % 10 + '0');
  return;
}

int n;
int p[7];

void perm(int i) {
  int j;
  int k;
  int t;

  if (i < n) {
    for (j = i; j < n; ++j) {
      t = p[i];
      p[i] = p[j];
      p[j] = t;
      perm(i + 1);
      t = p[i];
      p[i] = p[j];
      p[j] = t;
    }
  } else {
    for (k = 0; k < n; ++k) {
      putint(p[k]);
      putchar(' ');
    }
    putchar('\n');
  }
}

int main() {
  int i;
  n = 7;
  for (i = 0; i < n; i = i + 1) {
    p[i] = i;
  }
  perm(0);
  return 0;
}