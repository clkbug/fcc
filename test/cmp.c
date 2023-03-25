int main() {
  putchar('a' + (0 == 1));
  putchar('a' + (10 == 10));
  putchar('a' + (10 != 10));
  putchar('a' + (0 != 1));
  putchar('a' + (1 + 2 == 3));
  putchar('a' + (1 + 2 + 3 == 2 * 3));
  putchar('a' + (1 < 2));
  putchar('a' + (1 < 1));
  putchar('a' + (1 < 0));
  putchar('a' + (1 + 2 <= 3));
  putchar('a' + (1 + 2 < 3));
  putchar('a' + (5 > 6));
  putchar('a' + (2 + 3 > 2 * 3));
  putchar('a' + (7 <= 8));
  putchar('a' + (8 <= 8));
  putchar('a' + (9 <= 8));
  putchar('a' + (8 >= 7));
  putchar('a' + (2 * 4 >= 3 + 4));
  return 0;
}