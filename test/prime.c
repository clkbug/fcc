int putnum(int x) {
  if (x == 0) {
    putchar('0');
  } else {
    int i = 0;
    char buf[10];
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
  return 0;
}

int main() {
  int i = 2;
  while (i < 200) {
    int j = 2;
    while (j < i) {
      if (i % j == 0) {
        break;
      }
      j = j + 1;
    }
    if (j == i) {
      putnum(i);
      putchar('\n');
    }
    i = i + 1;
  }
  return 0;
}