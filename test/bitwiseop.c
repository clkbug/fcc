void putint(int x) {
  if (x >= 10) {
    putint(x / 10);
  }
  putchar(x % 10 + '0');
  return;
}

int main() {
  int i;
  int j;
  for (i = 0; i < 100; ++i) {
    for (j = 0; j < 100; ++j) {
      putint(i & j);
      putchar(',');
      putint(i | j);
      putchar(',');
      //   putint(~i);
      //   putchar(',');
      //   putint(~j);
      //   putchar(',');
      putint(i ^ j);
      putchar(';');
    }
    putchar('\n');
  }
  return 0;
}