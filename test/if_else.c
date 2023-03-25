int main() {
  if (1) {
    putchar('a');
  } else {
    putchar('b');
  }

  if (0) {
    putchar('c');
  } else {
    putchar('d');
  }

  if (1) {
    putchar('e');
  }

  if (0) {
    putchar('f');
  }

  if (1) putchar('g');
  if (0) putchar('h');

  if (1) {
    putchar('i');
  } else
    putchar('j');

  if (0)
    putchar('k');
  else {
    putchar('l');
    putchar('m');
  }

  if (2) {
    putchar('n');
  } else if (1) {
    putchar('o');
  } else {
    putchar('p');
  }
  return 0;
}