void f(int* p) { *p = *p + 1; }

int main() {
  int x = 1;
  int i = 0;
  for (i = 0; i < 10; i = i + 1) {
    f(&x);
    putchar('0' + x);
  }
  putchar('\n');

  return 0;
}