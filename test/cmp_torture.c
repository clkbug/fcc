int main() {
  char i;
  char j;
  for (i = 0; i < 127; i = i + 1) {
    for (j = 0; j < 127; j = j + 1) {
      putchar('0' + (i == j));
      putchar('0' + (i != j));
      putchar('0' + (i <= j));
      putchar('0' + (i < j));
      putchar('0' + (i >= j));
      putchar('0' + (i > j));
    }
  }
  return 0;
}