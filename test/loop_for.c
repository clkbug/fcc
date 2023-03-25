int main() {
  int a;
  int i;
  int j;
  a = 0;
  for (i = 0; i < 10; i = i + 1) {
    a = a + i;
  }
  putchar(a);
  a = 0;
  for (i = 0; i < 10; i = i + 1) {
    a = a + i;
    a = a + i;
  }
  putchar(a);
  for (i = 0; i < 10; i = i + 1) {
    putchar(a);
  }

  for (i = 0; i < 10; i = i + 1) {
    putchar('a' + i);
  }

  for (i = 0; i < 26; i = i + 1) {
    putchar('a' + i);
  }

  for (i = 0; i < 26; i = i + 1) {
    for (j = 0; j < i; j = j + 1) {
      putchar('a' + i);
    }
  }

  return 0;
}