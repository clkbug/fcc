int id(int x) { return x; }

int plus1(int x) { return x + 1; }

int plus(int x, int y) { return x + y; }

int sum(int x) {
  int i;
  int s;
  s = 0;
  for (i = 0; i < x; i = i + 1) {
    s = s + i;
  }
  return s;
}

int fact(int x) {
  int i;
  int s;
  s = 1;
  for (i = 1; i <= x; i = i + 1) {
    s = s * i;
  }
  return s;
}

int fib(int x) {
  if (x == 0) return 0;
  if (x == 1) return 1;
  return fib(x - 1) + fib(x - 2);
}

int main() {
  putchar('a' + id(0));
  putchar('a' + id(10));
  putchar('a' + id(20));
  putchar('\n');
  putchar('a' + plus1(0));
  putchar('a' + plus1(10));
  putchar('a' + plus1(20));
  putchar('\n');
  putchar('a' + plus(0, 1 + 2));
  putchar('a' + plus(10, 20 - 30 + 10));
  putchar('a' + plus(20, 1 + 2 * 3 - 5));
  putchar('\n');

  if (sum(3) == 6) putchar('a');
  if (sum(10) == 45) putchar('b');
  if (sum(100) == 4950) putchar('c');
  putchar('\n');

  if (fact(1) == 1) putchar('a');
  if (fact(3) == 6) putchar('b');
  if (fact(5) == 120) putchar('c');
  putchar('\n');

  if (fib(0) == 0) putchar('a');
  if (fib(1) == 1) putchar('b');
  if (fib(2) == 1) putchar('c');
  if (fib(3) == 2) putchar('d');
  if (fib(4) == 3) putchar('e');
  if (fib(5) == 5) putchar('f');
  if (fib(6) == 8) putchar('g');
  if (fib(7) == 13) putchar('h');
  if (fib(8) == 21) putchar('i');
  if (fib(9) == 34) putchar('j');
  if (fib(10) == 55) putchar('k');
  putchar('\n');

  return 0;
}