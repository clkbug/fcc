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

int main() {
  test0();

  return 0;
}