void f(int* p) { *p = *p + 1; }
void g(int** p) { **p = **p + 2; }

void test0() {
  int x = 1;
  int i = 0;
  int* p = &x;
  for (i = 0; i < 10; i = i + 1) {
    f(&x);
    putchar('0' + x);
  }
  putchar('\n');
  for (i = 0; i < 10; i = i + 1) {
    g(&p);
    putchar('0' + x);
  }
  putchar('\n');
}

int* h(int* p) { return p + 1; }

void test1() {
  int x[10];
  int i;
  int* p;
  for (i = 0; i < 10; i = i + 1) {
    x[i] = i;
  }
  for (i = 0; i < 9; i = i + 1) {
    p = h(&x[i]);
    putchar('0' + *p);
  }
}

int main() {
  test0();
  test1();
  return 0;
}