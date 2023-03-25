int test0() {
  int a[1];
  *a = 20;
  putchar('a' + *a);
  putchar('\n');
  return 0;
}

int test1() {
  int a[5];
  int i;
  int* p;
  for (i = 0; i < 5; i = i + 1) {
    a[i] = 2 * i + 1;
  }
  p = a;
  for (i = 0; i < 5; i = i + 1) {
    putchar('a' + *p);
    p = p + 1;
  }
  return 0;
}

int test2() {
  int a[10];
  int i;
  for (i = 0; i < 10; i = i + 1) {
    a[i] = i;
  }
  for (i = 0; i < 10; i = i + 1) {
    putchar('a' + a[i]);
  }
  putchar('\n');
  return 0;
}

int main() {
  test0();
  test1();
  test2();
  return 0;
}