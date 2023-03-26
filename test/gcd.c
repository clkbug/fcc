int gcd(int x, int y) {
  if (x < y) {
    return gcd(y, x);
  } else if (y == 0) {
    return x;
  } else {
    return gcd(y, x % y);
  }
}

void putint(int x) {
  int i = 0;
  char buf[10];
  if (x == 0) {
    putchar('0');
  } else {
    if (x < 0) {
      putchar('-');
      x = -x;
    }
    while (x > 0) {
      buf[i] = '0' + x % 10;
      x = x / 10;
      i = i + 1;
    }
    while (i > 0) {
      i = i - 1;
      putchar(buf[i]);
    }
  }
}

int main() {
  int i;
  int j;
  for (i = 1; i < 30; i = i + 1) {
    for (j = 1; j < 30; j = j + 1) {
      putint(gcd(i, j));
      putchar(' ');
    }
    putchar('\n');
  }
  return 0;
}