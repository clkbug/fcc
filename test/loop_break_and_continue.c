int test_while_break() {
  int i = 0;
  while (i < 10) {
    putchar('0' + i);
    if (i == 5) {
      break;
    }
    i = i + 1;
  }
  putchar('0' + i);
  putchar('\n');
  return 0;
}

int test_while_continue() {
  int i = 0;
  while (i < 10) {
    putchar('0' + i);
    if (i == 5) {
      i = i + 2;
      continue;
    }
    i = i + 1;
  }
  putchar('0' + i);
  putchar('\n');
  return 0;
}

int test_for_break() {
  int i;
  for (i = 0; i < 10; i = i + 1) {
    putchar('0' + i);
    if (i == 5) {
      break;
    }
  }
  putchar('0' + i);
  putchar('\n');
  return 0;
}

int test_for_continue() {
  int i;
  for (i = 0; i < 10; i = i + 1) {
    putchar('0' + i);
    if (i == 5) {
      i = i + 2;
      continue;
    }
  }
  putchar('0' + i);
  putchar('\n');
  return 0;
}

int test_double_for_loop() {
  int i;
  int j;
  for (i = 0; i < 20; i = i + 1) {
    if (i % 2 == 0) {
      continue;
    }

    for (j = 0; j < 20; j = j + 1) {
      if (j == 5) {
        continue;
      }
      putchar('!' + i);
      if (j == 10) {
        break;
      }
      putchar('!' + j);
      putchar('\n');
    }
    if (i > 10) {
      break;
    }
  }
  return 0;
}

int main() {
  test_while_break();
  test_while_continue();
  test_for_break();
  test_for_continue();
  test_double_for_loop();
  return 0;
}