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

int main() {
  int x = 10;
  array_of_int a;
  puts(s);
  test0(&x);
  putchar('\n');
  for (x = '0'; x < '9'; x = x + 1) {
    a[x] = x;
  }
  for (x = 0; x < 10; x = x + 1) {
    putchar(a[x]);
  }
  putchar('\n');

  test1();
  return 0;
}