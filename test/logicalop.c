int return_true() {
  putchar('t');
  return 1;
}
int return_false() {
  putchar('f');
  return 0;
}

void test_util() {
  return_true();
  return_false();
  putchar('\n');

  if (return_true()) {
    putchar('0');
  }
  if (return_false()) {
    putchar('1');
  }
  putchar('\n');
  return;
}

void test_and() {
  if (return_false() && return_false()) {
    putchar('a');
  }
  if (return_false() && return_true()) {
    putchar('b');
  }
  if (return_true() && return_false()) {
    putchar('c');
  }
  if (return_true() && return_true()) {
    putchar('d');
  }
  putchar('\n');
  return;
}

void test_andnot() {
  if (!return_false() && !return_false()) {
    putchar('a');
  }
  if (!return_false() && !return_true()) {
    putchar('b');
  }
  if (!return_true() && !return_false()) {
    putchar('c');
  }
  if (!return_true() && !return_true()) {
    putchar('d');
  }
  putchar('\n');
  return;
}

void test_and3() {
  if (return_false() && return_false() && return_false()) {
    putchar('a');
  }
  if (return_false() && return_false() && return_true()) {
    putchar('b');
  }
  if (return_false() && return_true() && return_false()) {
    putchar('c');
  }
  if (return_false() && return_true() && return_true()) {
    putchar('d');
  }
  if (return_true() && return_false() && return_false()) {
    putchar('e');
  }
  if (return_true() && return_false() && return_true()) {
    putchar('f');
  }
  if (return_true() && return_true() && return_false()) {
    putchar('g');
  }
  if (return_true() && return_true() && return_true()) {
    putchar('h');
  }
  putchar('\n');
  return;
}

void test_or() {
  if (return_false() || return_false()) {
    putchar('a');
  }
  if (return_false() || return_true()) {
    putchar('b');
  }
  if (return_true() || return_false()) {
    putchar('c');
  }
  if (return_true() || return_true()) {
    putchar('d');
  }
  putchar('\n');
  return;
}

void test_ornot() {
  if (!return_false() || !return_false()) {
    putchar('a');
  }
  if (!return_false() || !return_true()) {
    putchar('b');
  }
  if (!return_true() || !return_false()) {
    putchar('c');
  }
  if (!return_true() || !return_true()) {
    putchar('d');
  }
  putchar('\n');
  return;
}

void test_or3() {
  if (return_false() || return_false() || return_false()) {
    putchar('a');
  }
  if (return_false() || return_false() || return_true()) {
    putchar('b');
  }
  if (return_false() || return_true() || return_false()) {
    putchar('c');
  }
  if (return_false() || return_true() || return_true()) {
    putchar('d');
  }
  if (return_true() || return_false() || return_false()) {
    putchar('e');
  }
  if (return_true() || return_false() || return_true()) {
    putchar('f');
  }
  if (return_true() || return_true() || return_false()) {
    putchar('g');
  }
  if (return_true() || return_true() || return_true()) {
    putchar('h');
  }
  putchar('\n');
  return;
}

void test_andor3() {
  if (return_false() && return_false() || return_false()) {
    putchar('a');
  }
  if (return_false() && return_false() || return_true()) {
    putchar('b');
  }
  if (return_false() && return_true() || return_false()) {
    putchar('c');
  }
  if (return_false() && return_true() || return_true()) {
    putchar('d');
  }
  if (return_true() && return_false() || return_false()) {
    putchar('e');
  }
  if (return_true() && return_false() || return_true()) {
    putchar('f');
  }
  if (return_true() && return_true() || return_false()) {
    putchar('g');
  }
  if (return_true() && return_true() || return_true()) {
    putchar('h');
  }
  putchar('\n');
  return;
}

void test_orand3() {
  if (return_false() || return_false() && return_false()) {
    putchar('a');
  }
  if (return_false() || return_false() && return_true()) {
    putchar('b');
  }
  if (return_false() || return_true() && return_false()) {
    putchar('c');
  }
  if (return_false() || return_true() && return_true()) {
    putchar('d');
  }
  if (return_true() || return_false() && return_false()) {
    putchar('e');
  }
  if (return_true() || return_false() && return_true()) {
    putchar('f');
  }
  if (return_true() || return_true() && return_false()) {
    putchar('g');
  }
  if (return_true() || return_true() && return_true()) {
    putchar('h');
  }
  putchar('\n');
  return;
}

int main() {
  test_util();
  test_and();
  test_and3();
  test_or();
  test_or3();
  test_andor3();
  test_orand3();
  test_andnot();
  test_ornot();
  return 0;
}