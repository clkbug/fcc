int main() {
  int a;
  int b;
  int c;
  a = 1;
  b = 2;
  c = 3;
  putchar('a' + a);
  putchar('a' + b);
  putchar('a' + c);
  putchar('a' + a + b + c);
  a = b + c;
  b = a + c;
  c = a + b;
  putchar('a' + a);
  putchar('a' + b);
  putchar('a' + c);
  putchar('a' + a + b + c);
  putchar('a' + (a = 1 + 2 * 3));
  putchar('a' + (b = a + b - 1) + (c = 1 + 2 + 3));
  return 0;
}