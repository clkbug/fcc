struct struct_a {
  int a;
  int b;
};

void test0() {
  struct struct_a a;
  a.a = 10;
  a.b = 20;
  putchar(a.a + '0');
  putchar(a.b + '0');
  putchar('\n');
  return;
}

struct struct_b {
  int c;
  int b;
  int a;
};

void test1() {
  struct struct_b b;
  b.a = 10;
  b.b = 20;
  b.c = 30;
  putchar(b.a + '0');
  putchar(b.b + '0');
  putchar(b.c + '0');
  putchar('\n');
}

int main() {
  test0();
  test1();

  return 0;
}