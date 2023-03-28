typedef char* string;

string s = "hello, world";

typedef int* ptr_of_int;

void test0(ptr_of_int p) { *p = *p + 30; }

typedef int array_of_int[10];

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

  return 0;
}