int main() {
  int a;
  int b;
  int c;
  int *p;
  a = 1;
  b = 2;
  c = 3;
  p = &a;
  putchar('a' + *p);
  p = &b;
  putchar('a' + *p);
  p = &c;
  putchar('a' + *p);
  putchar('\n');
  return 0;
}