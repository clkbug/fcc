int test0() {
  int i = '0';
  ++i;
  putchar(i);
  putchar(++i);
  putchar(i);
  putchar(--i);
  putchar(i);
  return 0;
}

int test1() {
  int i = '0';
  int j = '0';
  ++i;
  putchar(i + j);
  putchar(++i + --j);
  putchar(++i + j);
  putchar(i + --j);
  putchar(--i + ++j);
  return 0;
}
int main() {
  test0();
  test1();
  return 0;
}
