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
  char c;
  int sum;
  int i;
  sum = 0;
  // check char overflow
  for (c = 0; c < 10; c = c + 257) {
    sum = sum + c;
  }
  putchar(sum);
  putchar('\n');
  return 0;
}

int test4() {
  char a[26];
  int i;
  // check int to char conversion
  for (i = 0; i < 26; i = i + 1) {
    a[i] = 'a' + i;
  }
  for (i = 0; i < 26; i = i + 1) {
    putchar(a[i]);
  }
  putchar('\n');
  return 0;
}

int test5() {
  char a[26];
  int i;
  for (i = 0; i < 26; i = i + 1) {
    a[i] = 'a' + i + 257;
  }
  for (i = 0; i < 26; i = i + 1) {
    putchar(a[i]);
  }
  putchar('\n');
  return 0;
}

int main() {
  test0();
  test1();
  test2();
  test3();
  test4();
  test5();
  return 0;
}