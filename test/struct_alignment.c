struct struct_a {
  char a;
  int b;
};

void test0() {
  struct struct_a a;
  struct struct_a* p = &a;

  a.a = 1024 + 10;
  a.b = 20;
  putchar(a.a + '0');
  putchar(a.b + '0');
  p->a = 1024 + 2048 + 22;
  p->b = 33;
  putchar(a.a + '0');
  putchar(a.b + '0');
  putchar(p->a + '0');
  putchar(p->b + '0');
  putchar('\n');
  return;
}

struct struct_b {
  char a[1];
  int b;
};

void test1() {
  struct struct_b a;
  struct struct_b* p = &a;

  a.a[0] = 55;
  (a.a)[1] = 1024 + 22;
  a.b = 33;
  putchar((a.a)[0]);
  putchar((a.a)[1] + '0');
  putchar(a.b + '0');
  putchar('\n');
  p->a[0] = 66;
  p->a[1] = 2048 + 1024 + 33;
  p->b = 44;
  putchar(a.a[0] + '0');
  putchar(a.a[1] + '0');
  putchar(a.b + '0');
  putchar(p->a[0] + '0');
  putchar(p->a[1] + '0');
  putchar(p->b + '0');
  putchar('\n');
  return;
}

struct struct_c {
  char a[4];
  int b;
};

void test2() {
  struct struct_c a;
  int i;
  for (i = 0; i < 4; i = i + 1) {
    a.a[i] = '0' + i;
  }
  a.b = 33;
  for (i = 0; i < 4; i = i + 1) {
    putchar(a.a[i]);
    putchar((a.a)[i]);
    putchar(a.b + '0');
  }
  putchar('\n');
  return;
}

int main() {
  test0();
  test1();
  test2();
  return 0;
}