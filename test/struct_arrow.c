struct struct_a {
  int a;
  int b;
};

void test0() {
  struct struct_a a;
  struct struct_a* p;
  p = &a;
  p->a = 10;
  p->b = 20;
  putchar(a.a + '0');
  putchar(a.b + '0');
  putchar(p->a + '0');
  putchar(p->b + '0');
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
  struct struct_b* p;
  p = &b;
  p->a = 10;
  p->b = 20;
  p->c = 30;
  putchar(b.a + '0');
  putchar(b.b + '0');
  putchar(b.c + '0');
  putchar(p->a + '0');
  putchar(p->b + '0');
  putchar(p->c + '0');
  putchar('\n');
}

int main() {
  test0();
  test1();

  return 0;
}