typedef char* string;

string s = "hello, world";

typedef int* ptr_of_int;

void test0(ptr_of_int p) { *p = *p + 30; }

typedef int array_of_int[10];

typedef ptr_of_int ptr_of_int2;
typedef ptr_of_int2* ptr_of_ptr_of_int2;

void test1() {
  int a;
  ptr_of_int p;
  ptr_of_ptr_of_int2 pp;
  p = &a;
  pp = &p;
  **pp = 10;
  putchar(a + '0');
  putchar(*p + '0');
  putchar(**pp + '0');
  putchar('\n');
}

typedef int* int_of_ptr;

int_of_ptr test2(int_of_ptr p, int_of_ptr q) {
  *p = *q + 1;
  *q = *p + 1;
  return q;
}

int main() {
  int x = 10;
  int y = 20;
  array_of_int a;
  puts(s);
  test0(&x);
  putchar('\n');
  for (x = 0; x < 10; x = x + 1) {
    a[x] = '0' + x;
  }
  for (x = 0; x < 10; x = x + 1) {
    putchar(a[x]);
  }
  putchar('\n');

  test1();

  test2(&x, &y);
  putchar(x + '0');
  putchar(y + '0');
  putchar('\n');
  return 0;
}