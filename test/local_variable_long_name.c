int main() {
  int first;
  int second;
  int third;
  first = 1;
  second = 2;
  third = 3;
  putchar('a' + first);
  putchar('a' + second);
  putchar('a' + third);
  putchar('a' + first + second + third);
  first = second + third;
  second = first + third;
  third = first + second;
  putchar('a' + first);
  putchar('a' + second);
  putchar('a' + third);
  putchar('a' + first + second + third);
  putchar('a' + (first = 1 + 2 * 3));
  putchar('a' + (second = first + second - 1) + (third = 1 + 2 + 3));
  return 0;
}