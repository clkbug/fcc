int f1() { return 1; }
int f100() { return 100; }

int inc(int a) { return a + 1; }

int plus(int a, int b) { return a + b; }

int sum(int n) {
  int s = 0;
  for (int i = 0; i <= n; i++) {
    s += i;
  }
  return s;
}

int fact(int n) {
  int p = 1;
  for (int i = 2; i <= n; i++) {
    p *= i;
  }
  return p;
}

int fib(int n) {
  if (n < 2) {
    return 1;
  } else {
    return fib(n - 1) + fib(n - 2);
  }
}