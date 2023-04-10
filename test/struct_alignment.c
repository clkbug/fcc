struct struct_a {
  char a;
  int b;
};

void test0() {
  struct struct_a a;
  a.a = 1024 + 10;
  a.b = 20;
  putchar(a.a + '0');
  putchar(a.b + '0');
  putchar('\n');
  return;
}

struct struct_b {
  char a[1];
  int b;
};

void test1() {
  struct struct_b a;
  a.a[0] = 55;
  (a.a)[1] = 1024 + 22;
  a.b = 33;
  putchar((a.a)[0]);
  putchar((a.a)[1] + '0');
  putchar(a.b + '0');
  putchar('\n');
  return;
}

int main() {
  test0();
  test1();
  return 0;
}