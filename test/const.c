const int A = 100;
const int B = 200;
const int C = 300;

void putint(int x) {
  if (x >= 10) {
    putint(x / 10);
  }
  putchar(x % 10 + '0');
  return;
}

int main() {
  const int D = 400;
  const int E = 500;
  const int F = 600;
  putint(A);
  putchar('\n');
  putint(B);
  putchar('\n');
  putint(C);
  putchar('\n');
  putint(D);
  putchar('\n');
  putint(E);
  putchar('\n');
  putint(F);
  putchar('\n');
  return 0;
}