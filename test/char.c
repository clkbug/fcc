int test0() {
  char a;
  for (a = 'a'; a <= 'z'; a = a + 1) {
    putchar(a);
  }
  putchar('\n');
  return 0;
}

int test1() {
  char a[4];
  a[0] = 'a';
  putchar(a[0]);
  a[1] = 'b';
  putchar(a[1]);
  a[2] = 'c';
  putchar(a[2]);
  a[3] = 'd';
  putchar(a[3]);
  putchar('\n');

  putchar(a[0]);
  putchar(a[1]);
  putchar(a[2]);
  putchar(a[3]);
  putchar('\n');

  return 0;
}

int test2() {
  char a[3];
  a[0] = 'a';
  putchar(a[0]);
  a[1] = 'b';
  putchar(a[1]);
  a[2] = 'c';
  putchar(a[2]);
  putchar('\n');

  putchar(a[0]);
  putchar(a[1]);
  putchar(a[2]);
  putchar('\n');

  return 0;
}

int test3() {
  char a[26];
  int i;
  for (i = 0; i < 26; i = i + 1) {
    a[i] = 'a' + i;
  }
  for (i = 0; i < 26; i = i + 1) {
    putchar(a[i]);
  }
  return 0;
}

int main() {
  test0();
  test1();
  test2();
  return 0;
}