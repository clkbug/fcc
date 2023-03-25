int a;
int b;

int test0() {
  a = 1;
  b = 2;
  putchar('a' + a);
  putchar('a' + b);
  putchar('\n');
  return 0;
}

int array[10];

int test1() {
  int i;
  for (i = 0; i < 10; i = i + 1) {
    array[i] = i * 2 + 1;
  }
  for (i = 0; i < 10; i = i + 1) {
    putchar('a' + array[i]);
  }
  putchar('\n');
  return 0;
}

int main() {
  test0();
  test1();
  return 0;
}